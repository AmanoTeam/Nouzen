#!/bin/bash

declare -r APP_DIRECTORY="$(realpath "$(( [ -n "${BASH_SOURCE}" ] && dirname "$(realpath "${BASH_SOURCE[0]}")" ) || dirname "$(realpath "${0}")")")"

declare action
declare packages
declare args

while [[ $# -gt 0 ]]; do
	
	if [ "${1}" = 'update' ]; then
		action='--update'
	elif [ "${1}" = 'install' ]; then
		action='--install'
	elif [ "${1}" = 'destroy' ]; then
		action='--destroy'
	elif [ "${1}" = 'remove' ] || [ "${1}" = 'uninstall' ] || [ "${1}" = 'autoremove' ] || [ "${1}" = 'purge' ]; then
		action='--uninstall'
	elif [[ "${1}" == '-'* ]]; then
		args+="${1} "
	else
		packages+="${1};"
	fi
	
  shift
done

if [ -n "${packages}" ]; then
	"${APP_DIRECTORY}/nz" ${args} ${action} "${packages}"
else
	"${APP_DIRECTORY}/nz" ${args} ${action}
fi

exit "${?}"
