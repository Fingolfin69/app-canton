#!/bin/bash

# Simple nanopb generation script that avoids generator modification
set -e -o pipefail

# Define color functions
red() { echo -e "\e[31m$*\e[0m"; }
green() { echo -e "\e[32m$*\e[0m"; }
yellow() { echo -e "\e[33m$*\e[0m"; }

# Setup paths - same as your original script
if git rev-parse --is-inside-work-tree &>/dev/null || false; then
    green "Running inside a git repository, using absolute paths."
    ROOT_PATH=$(git rev-parse --show-toplevel)
    LEDGER_API_PROTO_PATH=$ROOT_PATH/canton/community/ledger-api/src/main/protobuf
    LAPI_VALUE_PROTO_PATH=$ROOT_PATH/daml/sdk/daml-lf/ledger-api-value/src/main/protobuf/com/daml/ledger/api/v2/value.proto
else
    green "Running outside a git repository, using relative paths."
    ROOT_PATH=../../protobuf
    LEDGER_API_PROTO_PATH=$ROOT_PATH/ledger-api
    LAPI_VALUE_PROTO_PATH=$LEDGER_API_PROTO_PATH/com/daml/ledger/api/v2/value.proto
fi

LEDGER_API_V2_PATH=$LEDGER_API_PROTO_PATH/com/daml/ledger/api/v2
OUTPUT_DIR="./src/"
NANOPB_GENERATOR="nanopb/generator/protoc-gen-nanopb"

mkdir -p "$OUTPUT_DIR"

# Download utility
download_if_not_exists() {
  local url=$1
  local file_path=$2
  if [ ! -f "$file_path" ]; then
    echo "Downloading $file_path"
    mkdir -p "$(dirname "$file_path")"
    curl -s "$url" -o "$file_path"
  fi
}

# Define function that does the previous if / else logic
clone_if_not_exists() {
  local repo_url=$1
  local sparse_path=$2
  local repo_name=$(basename "$repo_url" .git)

  if [ ! -d "$repo_name" ]; then
    yellow "Cloning $repo_name repository..."
    git clone --filter=blob:none --sparse "$repo_url" && \
    cd "$repo_name" && \
    git sparse-checkout set "$sparse_path"
    cd ..
  else
    green "$repo_name repository already cloned."
  fi
}

# Setup dependencies
echo "Setting up dependencies..."
download_if_not_exists "https://raw.githubusercontent.com/protocolbuffers/protobuf/407aa2d9319f5db12964540810b446fecc22d419/src/google/protobuf/empty.proto" "google/protobuf/empty.proto"
download_if_not_exists "https://raw.githubusercontent.com/googleapis/googleapis/3597f7db2191c00b100400991ef96e52d62f5841/google/rpc/status.proto" "google/rpc/status.proto"
download_if_not_exists "https://raw.githubusercontent.com/googleapis/googleapis/9415ba048aa587b1b2df2b96fc00aa009c831597/google/rpc/error_details.proto" "google/rpc/error_details.proto"
download_if_not_exists "https://raw.githubusercontent.com/protocolbuffers/protobuf/refs/heads/main/src/google/protobuf/any.proto" "google/protobuf/any.proto"
download_if_not_exists "https://raw.githubusercontent.com/protocolbuffers/protobuf/refs/heads/main/src/google/protobuf/duration.proto" "google/protobuf/duration.proto"
download_if_not_exists "https://raw.githubusercontent.com/protocolbuffers/protobuf/refs/heads/main/src/google/protobuf/timestamp.proto" "google/protobuf/timestamp.proto"

clone_if_not_exists "https://github.com/digital-asset/daml.git" "sdk/daml-lf/ledger-api-value/src/main/protobuf/com/daml/ledger/api/v2"
clone_if_not_exists "https://github.com/digital-asset/canton.git" "community/ledger-api/src/main/protobuf/com/daml/ledger/api/v2/interactive"
mkdir -p "com/daml/ledger/api/v2" && cp "$LAPI_VALUE_PROTO_PATH" "com/daml/ledger/api/v2/value.proto"

# Create the options file for value.proto
echo "Creating value.options file..."
cat > value.options << 'EOF'
# Handle recursive Value fields with pointers to break cycles
com.daml.ledger.api.v2.RecordField.value type:FT_POINTER
com.daml.ledger.api.v2.List.elements type:FT_POINTER  
com.daml.ledger.api.v2.Optional.value type:FT_POINTER
com.daml.ledger.api.v2.Variant.value type:FT_POINTER
com.daml.ledger.api.v2.TextMap.Entry.value type:FT_POINTER
com.daml.ledger.api.v2.GenMap.Entry.value type:FT_POINTER
com.daml.ledger.api.v2.GenMap.Entry.key type:FT_POINTER

# Static allocation for string fields to avoid callback issues in oneof
com.daml.ledger.api.v2.Value.numeric type:FT_STATIC max_size:64
com.daml.ledger.api.v2.Value.party type:FT_STATIC max_size:256
com.daml.ledger.api.v2.Value.text type:FT_STATIC max_size:1024
com.daml.ledger.api.v2.Value.contract_id type:FT_STATIC max_size:256
com.daml.ledger.api.v2.Identifier.package_id type:FT_STATIC max_size:256
com.daml.ledger.api.v2.Identifier.module_name type:FT_STATIC max_size:256
com.daml.ledger.api.v2.Identifier.entity_name type:FT_STATIC max_size:256
com.daml.ledger.api.v2.Variant.constructor type:FT_STATIC max_size:128
com.daml.ledger.api.v2.Enum.constructor type:FT_STATIC max_size:128
com.daml.ledger.api.v2.RecordField.label type:FT_STATIC max_size:128
com.daml.ledger.api.v2.TextMap.Entry.key type:FT_STATIC max_size:256
EOF

# Generate nanopb C/H code for protobuf messages
generate_nanopb_code() {
  local include_paths=$1
  local proto_file=$2
  local extra_opts=${3:-""}

  yellow "Generating nanopb C/H code for $proto_file"

  # Extract the base name for the options file
  local base_name=$(basename "$proto_file" .proto)
  local options_file="${base_name}.options"
  
  # Check if specific options file exists, otherwise use value.options for Value-related protos
  if [ ! -f "$options_file" ] && [[ "$proto_file" == *"value.proto"* ]]; then
    options_file="value.options"
  fi

  # Build the protoc command
  local protoc_cmd="protoc --nanopb_out=$OUTPUT_DIR"
  
  # Add options file if it exists
  if [ -f "$options_file" ]; then
    protoc_cmd="$protoc_cmd --nanopb_opt=-f$options_file"
  fi
  
  # Add common options to handle recursion and static allocation
  protoc_cmd="$protoc_cmd --nanopb_opt=-T"
  protoc_cmd="$protoc_cmd --nanopb_opt=-s\"max_size:1024\""
  
  # Add extra options if provided
  if [ -n "$extra_opts" ]; then
    protoc_cmd="$protoc_cmd $extra_opts"
  fi
  
  # Add include paths
  protoc_cmd="$protoc_cmd -I$include_paths -I. --plugin=protoc-gen-nanopb=$NANOPB_GENERATOR $proto_file"
  
  # Execute the command
  yellow "Running: $protoc_cmd"
  eval $protoc_cmd
}

# Generate nanopb C/H code
echo "Generating nanopb C/H code from protobuf definitions..."

# Replace "bool" and "enum" field names in value.proto to make sure only C allowed names are used.
sed -i -E 's/\bbool bool\b/bool bool_/g; s/\bEnum enum\b/Enum enum_/g' com/daml/ledger/api/v2/value.proto

# Generate value.proto first to ensure all dependencies are available
generate_nanopb_code "." "com/daml/ledger/api/v2/value.proto"

# Generate essential proto files
generate_nanopb_code "." "google/protobuf/empty.proto"
generate_nanopb_code "." "google/rpc/status.proto"
generate_nanopb_code "." "google/rpc/error_details.proto"
generate_nanopb_code "." "google/protobuf/any.proto"
generate_nanopb_code "." "google/protobuf/duration.proto"
generate_nanopb_code "." "google/protobuf/timestamp.proto"

# Generate main interactive submission service
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/interactive/interactive_submission_service.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/interactive/interactive_submission_common_data.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/interactive/transaction/v1/interactive_submission_data.proto"

# Generate other ledger API files
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/offset_checkpoint.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/package_reference.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/trace_context.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/commands.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/completion.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/event.proto"
generate_nanopb_code "$LEDGER_API_PROTO_PATH" "$LEDGER_API_V2_PATH/transaction.proto"

green "Done! Generated files are in: $OUTPUT_DIR"
