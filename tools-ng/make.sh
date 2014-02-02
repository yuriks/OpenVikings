#!/bin/sh

for tool in src/lvt-*; do
	rustpkg install $(basename $tool)
done
