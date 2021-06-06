#!/bin/sh
set -e # exit on error

function github_clone {
	local USER=$1
	local REPO=$2
	local REV=$3
	echo Fetching https://github.com/$USER/$REPO.git at $REV
	[ -e $REPO ] && rm -rf $REPO
	curl -o $REPO.zip -s -L https://github.com/$USER/$REPO/archive/$REV.zip
	unzip -q $REPO.zip
	rm $REPO.zip
	mv $REPO-$REV $REPO
}

github_clone glfw    glfw  114776a24605418e6d719d2f30141e351e93c6e0
github_clone ocornut imgui a640d8a6d2bfd2e8f125c1dc72594543808ecddf
