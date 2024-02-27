#!/bin/sh

find . -name "*.exe" -type f -delete

git pull
git add *
git commit -m 'update'
git push
