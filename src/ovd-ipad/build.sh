#!/bin/bash

#set -x

PROJDIR=$(cd $(dirname $0); pwd -P)
WORKSPACE_NAME=$(echo *.xcworkspace)
TARGET_SDK="iphoneos"
SCHEMES="OVDiPad"
PROJECT_BUILDDIR="${PROJDIR}/build"
BUILD_HISTORY_DIR="${PROJDIR}/build/output"

. ${PROJDIR}/.buildconfig

# compile project
echo Building Project
cd "${PROJDIR}"

security list-keychains -s "${KEYCHAIN}"
security unlock-keychain -p "${KEYCHAIN_PASSWORD}" "${KEYCHAIN}"

for SCHEME in ${SCHEMES}; do
	xcodebuild build \
		-verbose \
		-workspace "${WORKSPACE_NAME}" \
		-scheme "${SCHEME}" \
		-sdk "${TARGET_SDK}" \
		-configuration "${CONFIGURATION}" \
		SYMROOT="${PROJECT_BUILDDIR}" \
		CONFIGURATION_BUILD_DIR="${PROJECT_BUILDDIR}/${CONFIGURATION}-${TARGET_SDK}" \
		CODE_SIGN_IDENTITY="${CODE_SIGN_IDENTITY}" \
		PROVISONNING_PROFILE="${PROVISONNING_PROFILE}" \
		PROJECT_TEMP_DIR="${PROJECT_BUILDDIR}/${CONFIGURATION}-${TARGET_SDK}/tmp"

	#Check if build succeeded
	if [ $? != 0 ]
	then
		exit 1
	fi
done

mkdir -p "${BUILD_HISTORY_DIR}"
/usr/bin/xcrun \
	-sdk "${TARGET_SDK}" \
	PackageApplication \
	-v "${PROJECT_BUILDDIR}/${CONFIGURATION}-${TARGET_SDK}/ovd2013.app" \
	-o "${BUILD_HISTORY_DIR}/${APPLICATION_NAME}-$(date '+%y%m%d').ipa" \
	--sign "${CODE_SIGN_IDENTITY}" \
	--embed "${PROVISONNING_PROFILE}"

security list-keychains -s ${KEYCHAINS}
