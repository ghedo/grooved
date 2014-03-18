# groovectl bash completion
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

_groovectl() {
	local cur prev

	COMPREPLY=()

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	CMDS="status play pause toggle next prev add rgain loop lyrics quit"

	COMPREPLY=($(compgen -W "$CMDS" -- $cur))

	return 0
}
complete -f -F _groovectl groovectl
