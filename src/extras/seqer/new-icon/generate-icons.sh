#!/bin/bash

input_filename=$1
output_base_filename=$2

# Windows
convert "$input_filename" -define icon:auto-resize=128,64,48,32,20,16 "$output_base_filename.ico"

# Mac
mkdir "$output_base_filename.iconset"

for size in {16,32,64,128,256,512}
do
	convert "$input_filename" -resize ${size}x${size} "$output_base_filename.iconset/icon_${size}x${size}.png"
	convert "$input_filename" -resize $((size * 2))x$((size * 2)) "$output_base_filename.iconset/icon_${size}x${size}@2x.png"
done

iconutil --convert icns "$output_base_filename.iconset" --output "$output_base_filename.icns"
rm -rf "$output_base_filename.iconset"

# Unix
convert "$input_filename" -resize 256x256 "${output_base_filename}.xpm"
mv "$output_base_filename.xpm" $output_base_filename.xpm.tmp
sed -e 's/^static char \*\([^[ ]*\)/static const char * \1_xpm/' < $output_base_filename.xpm.tmp > $output_base_filename.xpm
rm $output_base_filename.xpm.tmp

