#!/bin/zsh

#
# Splits a Markdown-like file into multiple subfiles based on '##' headings.
# Each subfile is named after its heading.
#

# --- Configuration ---
# Set to true to see more detailed output during execution.
local VERBOSE=true

# --- Function Definitions ---

# Displays usage information and exits.
usage() {
  echo "Usage: $0 <input_file>"
  echo "Splits a file into multiple parts based on lines starting with '## '."
  exit 1
}

# --- Pre-flight Checks ---

# 1. Ensure the 'csplit' utility is available.
if ! command -v csplit &> /dev/null; then
  echo "Error: 'csplit' command not found. This script requires it to function." >&2
  echo "On macOS, install with: brew install coreutils" >&2
  echo "On Debian/Ubuntu, install with: sudo apt-get install coreutils" >&2
  exit 1
fi

# 2. Check for the correct number of arguments.
if [[ $# -ne 1 ]]; then
  usage
fi

# 3. Verify the input file exists and is readable.
local input_file="$1"
if [[ ! -f "$input_file" ]]; then
  echo "Error: Input file not found at '$input_file'." >&2
  exit 1
fi
if [[ ! -r "$input_file" ]]; then
  echo "Error: Input file is not readable. Check permissions for '$input_file'." >&2
  exit 1
fi

# --- Main Logic ---

# Create a secure, temporary directory to work in. This prevents clutter
# and avoids overwriting existing files until the very end.
local temp_dir
temp_dir=$(mktemp -d "${TMPDIR:-/tmp}/split_sh.XXXXXX")

# The 'trap' command is a safeguard. It ensures that no matter how the script
# exits (normally, with an error, or via interruption), the temporary
# directory will be automatically and safely removed.
trap 'rm -rf "$temp_dir"' EXIT INT TERM

$VERBOSE && echo "Created temporary directory at '$temp_dir'"

# Use csplit to break the file into pieces.
#  -s : Runs in silent mode.
#  -k : Keeps the generated files even if an error occurs.
#  -z : Elides (removes) empty output files. This is crucial to avoid creating
#       an empty file from the content before the very first '##' delimiter.
#  -f : Specifies the prefix for the output files.
# The pattern '/^## /' tells csplit where to split.
# '{*}' repeats the split for every occurrence of the pattern.
if ! csplit -skz -f "$temp_dir/part_" "$input_file" '/^## /' '{*}'; then
    echo "Error: csplit failed to split the file. Aborting." >&2
    exit 1 # The trap will handle cleanup
fi

$VERBOSE && echo "Successfully split file into temporary parts."

# --- File Renaming ---

# Loop through each temporary part file created by csplit.
for part_file in "$temp_dir"/part_*; do
  # A safety check in case an empty or unreadable part was created.
  [[ -s "$part_file" ]] || continue

  # Read the very first line of the part file to get the section title.
  local title
  title=$(head -n 1 "$part_file")

  # Sanitize the title to create a safe and valid filename.
  # 1. Remove the leading '## ' and any extra whitespace.
  # 2. Replace troublesome characters (spaces, slashes) with a hyphen.
  # 3. Remove any character that is not alphanumeric, a hyphen, or a dot.
  # 4. Append the '.md' file extension.
  local new_filename
  new_filename=$(echo "$title" |
    sed -e 's/^## \s*//' \
        -e 's/[[:space:]\/]/-/g' \
        -e 's/[^a-zA-Z0-9.-]//g' \
        -e 's/$/.md/')

  # If the sanitization results in an empty filename, skip it.
  if [[ -z "$new_filename" ]]; then
      echo "Warning: Could not generate a valid filename from title: '$title'. Skipping." >&2
      continue
  fi

  # Move the temporary part from its temp directory into the current
  # directory with its new, meaningful name.
  mv "$part_file" "./$new_filename"
  echo "Created: ./$new_filename"
done

echo "\n✅ Splitting complete."

# The trap automatically cleans up the temp directory, so no further action is needed.
exit 0
