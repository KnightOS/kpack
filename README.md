# kpg

The KnightOS package manager.

## On-Calc

The on-calc KnightOS package manager is called `kpm`, and can be used to install
packages by setting A to the desired operation:

* 0x00: Install a package
* 0x01: Remove a package

### Package installation

Set registers to these values to install a package:

* DE: Path to package
* HL: Progress memory (see below)
* B: Flags (bitfield)
  * 0: Reset to prevent dependency discovery

### Package removal

Set registers to these values to remove a package:

* DE: Package name
* HL: Progress memory (see below)

### Progress reporting

Allocate 3 bytes of memory through which kpm will report progress to you. It will
look something like this:

* Overall progress (8-bit unsigned integer, 0-255 where 255 is "done")
* Current operation name (16-bit string pointer)

# Compiling

To compile the on-calc software, `cd knightos` and run `make`.

To compile the computer software, run `cmake .` and then `make`. Install with
`make install` as root.
