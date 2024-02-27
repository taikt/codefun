#!/bin/sh

find . -name "*.exe a.out" -type f -delete

git pull
git add *
git commit -m 'update'
git push
