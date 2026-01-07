# Placer Based on the "Simulated Annealing" Algorithm

A standard cell placer for both global and detailed placement, based on a modified "simulated annealing" algorithm for VLSI and FPGA design.

This source code was once part of my master's thesis, where I explored the simulated annealing algorithm with certain modifications for placing standard cells on an integrated circuit. I never found the time to refactor and eliminate bad practices, but I used to compile this with VS2013-VS2015 on Windows 7 and it worked with [IBM-PLACE Benchmarks] in the Bookshelf format. Since I haven't had experience with C++ for over 9 years, I don't see the point in spending time on refactoring or making improvements now. I'm leaving it here as it was, hoping someone might find it useful.

Some logic as I remember was borrowed from [TimberWolf], [UMpack] and [UCLApack].

## Building

### Requirements
- C++11 compatible compiler (clang++ or g++)
- OpenGL and GLUT frameworks (available on macOS, may need freeglut on Linux)

### Build Instructions

```bash
make clean
make
```

The executable `siman` will be created in the project directory.

## Usage

```bash
./siman -f <input.aux> [options]
```

### Options
- `-f filename.aux` - Input file in Bookshelf format (required)
- `-save filename.pl` - Output placement file (default: `ibm_SA_out.pl`)
- `-detailed` - Use detailed placement mode (default: global placement)
- `-greedy` - Greedy mode without annealing (faster, less optimal)
- `-skipLegal` - Skip legalization step
- `-help` or `-h` - Show help message

### Example

```bash
./siman -f benchmark.aux -save output.pl -detailed
```

## Input Format

The program uses the **Bookshelf format**, which consists of:
- `.aux` - Master file listing all other files
- `.nodes` - Cell definitions
- `.nets` - Netlist (connections)
- `.scl` - Row definitions
- `.pl` - Initial placement (optional)

All files must be in the same directory as the `.aux` file.

## Output

The program generates:
- Console output with placement statistics (HPWL, overlaps, execution time)
- Placement file (`.pl`) with final cell positions
- Optional OpenGL visualization window

## Test Data

The program was designed to work with **IBM-PLACE Benchmarks**:
- [IBM-PLACE Benchmarks](http://vlsicad.eecs.umich.edu/BK/Slots/cache/er.cs.ucla.edu/benchmarks/ibm-place/)

## Porting Notes

This code has been ported from Windows (VS2013-VS2015) to macOS/Linux

## References

- [IBM-PLACE Benchmarks]: http://vlsicad.eecs.umich.edu/BK/Slots/cache/er.cs.ucla.edu/benchmarks/ibm-place/
- [UMpack]: http://vlsicad.eecs.umich.edu/BK/PDtools/tar.gz/LATEST/UMpack-45-120708.tar.gz
- [UCLApack]: ./UCLApack-43-040113.tar.gz_
- [TimberWolf]: https://github.com/rubund/graywolf/blob/master/README.md
