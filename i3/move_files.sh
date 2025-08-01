#!/usr/bin/env bash

# Exit on error, unset variable reference, and pipeline failures
set -euo pipefail

cleanup() {
    echo "Cleaning up and exiting..."
    kill $inotify_pid 2>/dev/null || true
    exit "${1:-0}"
}

# Set up signal traps for graceful termination
trap 'cleanup 0' EXIT
trap 'cleanup 1' HUP INT QUIT TERM

if [ $# -lt 2 ]; then
    echo "Usage: $0 <download_dir> <books_dir>" >&2
    exit 1
fi

download_dir="$1"
books_dir="$2"

# Validate directories exist and are accessible
if [ ! -d "$download_dir" ]; then
    echo "Error: Download directory '$download_dir' does not exist" >&2
    exit 1
fi

if [ ! -d "$books_dir" ]; then
    echo "Error: Books directory '$books_dir' does not exist" >&2
    exit 1
fi

if [ ! -w "$books_dir" ]; then
    echo "Error: Books directory '$books_dir' is not writable" >&2
    exit 1
fi

# Supported file extensions
valid_extensions=("pdf" "epub" "djvu")

# Function to check if extension is valid
is_valid_extension() {
    local extension="${1,,}" # Convert to lowercase
    for valid_ext in "${valid_extensions[@]}"; do
        [ "$extension" == "$valid_ext" ] && return 0
    done
    return 1
}

# Check if inotifywait is installed
if ! command -v inotifywait >/dev/null 2>&1; then
    echo "Error: inotifywait not installed. Please install inotify-tools package." >&2
    exit 1
fi

declare -A pending_files
# Auto-cleanup for pending_files older than 30 minutes
cleanup_pending_files() {
    local now=$(date +%s)
    for file in "${!pending_files[@]}"; do
        if ((now - pending_files["$file"] > 1800)); then
            unset pending_files["$file"]
        fi
    done
}

# Start inotifywait in background
inotifywait -m -e create,close_write,move --format '%e %f' "$download_dir" | while read -r event file_name; do
    # Run cleanup every 100 events
    ((event_count++ % 100 == 0)) && cleanup_pending_files
    
    case "$event" in
        CREATE)
            pending_files["$file_name"]=$(date +%s)
            ;;
        CLOSE_WRITE)
            if [ -v pending_files["$file_name"] ]; then
                full_path="$download_dir/$file_name"
                if [ -f "$full_path" ]; then
                    # Extract extension safely
                    if [[ "$file_name" == *.* ]]; then
                        extension="${file_name##*.}"
                        if is_valid_extension "$extension"; then
                            target_path="$books_dir/$file_name"
                            if [ -e "$target_path" ]; then
                                echo "Warning: File already exists at destination. Renaming to ${file_name%.*}_$(date +%s).${extension}"
                                target_path="$books_dir/${file_name%.*}_$(date +%s).${extension}"
                            fi
                            if mv "$full_path" "$target_path"; then
                                echo "Successfully moved $file_name to books directory"
                            else
                                echo "Failed to move $file_name" >&2
                            fi
                        fi
                    fi
                fi
                unset pending_files["$file_name"]
            fi
            ;;
        MOVED_FROM)
            if [ -v pending_files["$file_name"] ]; then
                unset pending_files["$file_name"]
            fi
            ;;
        *)
            # Log unexpected events
            echo "Unhandled event: $event for file: $file_name" >&2
            ;;
    esac
done &

inotify_pid=$!
wait $inotify_pid
