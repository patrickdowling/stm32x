#!/bin/sh -e

# Pull relevant parts from CMSIS repo

API_URL=https://api.github.com/repos/arm-software/CMSIS_5/releases/latest
REPO_URL=https://github.com/arm-software/CMSIS_5.git

TAG=$(curl -s $API_URL | python3 -c "import sys, json; print(json.load(sys.stdin)['tag_name'])")

CMSIS_SRC_DIR=./tmp
CMSIS_DST_DIR=./CMSIS

git clone -c advice.detachedHead=false -q --depth 1 --branch "$TAG" "$REPO_URL" "$CMSIS_SRC_DIR"

rm -rf "$CMSIS_DST_DIR"
mkdir -p "$CMSIS_DST_DIR"

SUBDIRS=("Core/Include" "DSP/Include" "DSP/PrivateInclude" "DSP/Source")

for subdir in ${SUBDIRS[@]}; do
	mkdir -p $CMSIS_DST_DIR/$subdir
	cp -R $CMSIS_SRC_DIR/CMSIS/$subdir/* $CMSIS_DST_DIR/$subdir
done

cp $CMSIS_SRC_DIR/LICENSE.txt $CMSIS_DST_DIR

echo "$TAG" | tee $CMSIS_DST_DIR/cmsis_version

cat << EOF > $CMSIS_DST_DIR/README.md
$(date): Extracted from $REPO_URL at tag $TAG
EOF


rm -rf "$CMSIS_SRC_DIR"
