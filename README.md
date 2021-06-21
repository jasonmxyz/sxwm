# SXWM: Superior X Window Manager

SXWM is a window manager for X inspired by DWM, i3, and other tiling window
managers. Windows are assigned to tags, and are automatically arranged on the
screen in a stacked layout making the most of your monitor. SXWM is easily
configurable in the config file and is designed to be as extensible as
possible.

Created by [Jason Moore](https://github.com/jasonmxyz/)

## Building

The required build tools are: `gcc`, `make`
You will also require the necessary libraries `glibc`, `libx11`, `xorgproto`

    make sxwm
	make sxwmbar

## Usage

You can test SXWM using make by running

    make test

If you're feeling brave you can add SXWM to your `.xinitrc` and see just how
unfinished it is.

    sxwm [--help] [--config </path/to/config>]

## Configuration

SXWM is capable of reading from a config file specified as a command line
argument. Use the `-c` or `--config` command line option to specify a
configuration file. There is an example config file in `share/defaultconfig`
which you could use as a start. It shows off the commands available.

    sxwm -c ./share/defaultconfig
	sxwm --config=./share/defaultconfig

Comments in the configuration file are denoted with `#` symbols.

    # This line is a comment
	run some program	# This is a comment at the end of a line

Bind a key combination to an internal function with the `bind` operation.

    bind    win+1        selectTag 1
	bind    win+shift+q  exit

Use the `run` and `start` operations to execute another program in the same or
in a new session.

    run   ./sxwmbar        # Will stop running when you close SXWM
	start another program  # Will continue running after SXWM finishes
