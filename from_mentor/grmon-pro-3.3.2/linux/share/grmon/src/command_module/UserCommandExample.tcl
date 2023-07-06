
namespace eval MyModule {

    proc dummycmd {args} {
    foreach a $args {
        puts $a
    }
    }

    set foo {"Thinking of Emacs as just an editor is like thinking of the space\
        shuttle as just another part of the public transportation system."\
        (Dave Kenny/comp.emacs)}

    proc loadtruth {} {
    variable foo

        wmems 0x40010000 $foo
    }

    proc incrPC {} {
        # Use the command silent to suppress command output for a single command
        # The command reg is used to get/set cpu registers
        set pc [silent reg pc]
        silent reg pc [expr $pc+4]

	# Set grmon_suppress_output variable to suppress output for several commands
	set old_suppress [expr $::grmon::settings::suppress_output & 5]
	set ::grmon::settings::suppress_output [expr $::grmon::settings::suppress_output | 5]

        set pc [reg pc]
        reg npc [expr $pc+4]

	set ::grmon::settings::suppress_output [expr ($::grmon::settings::suppress_output & ~5) | $old_suppress]
    }

    proc MyInit {} {
        # this will be executed in all shells
        puts "==> Init User Commands"

        if {$::grmon_shell == "usr1"} {
            # This will only be executed in usr1 shell (i.e. the terminal shell)
            puts "==> Init User Commands 2"
        }
    }

    proc MyExit {} {
        # this will be executed in all shells
        puts "==> Init User Commands"

        if {$::grmon_shell == "usr1"} {
            # This will only be executed in usr1 shell (i.e. the terminal shell)
            puts "==> Init User Commands 2"
        }
    }

}

# Create alias to global namespace
interp alias {} MyModule::dummycmd {} dummycmd
interp alias {} MyModule::loadtruth {} loadtruth
interp alias {} MyModule::incrPC {} incrPC

# Initialize module
MyModule::MyInit
lappend ::hooks::closedown MyModule::MyExit
