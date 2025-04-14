#!/bin/sh

find . -name "*.exe" -type f -delete
find . -name "a.out" -type f -delete

git pull
git add -u .
git commit -m 'update'
git push
