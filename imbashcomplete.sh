#!/bin/bash


if ! [[ $1 =~ ^[0-9]{6}$ ]]; then 
	echo "error, not 6 digit input" >&2 
	exit 1 
elif ! grep -q $1 nanpa; then 
	echo "error, phone number not found" >&2
	exit 1
else grep $1 nanpa | sed s/$1// | sed s/" "[A-Z][A-Z]// 
fi


