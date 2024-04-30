# Placer Based on the "Simulated Annealing" Algorithm

A standard cell placer for both global and detailed placement, based on a modified "simulated annealing" algorithm for VLSI and FPGA design.

This source code was once part of my master's thesis, where I explored the simulated annealing algorithm with certain modifications for placing standard cells on an integrated circuit. I never found the time to refactor and eliminate bad practices, but I used to compile this with VS2013-VS2015 on Windows 7 and it worked with [IBM-PLACE Benchmarks] in the Bookshelf format. Since I haven't had experience with C++ for over 9 years, I don't see the point in spending time on refactoring or making improvements now. I'm leaving it here as it was, hoping someone might find it useful.

Some logic as I remember was borrowed from [TimberWolf], [UMpack] and [UCLApack].

[IBM-PLACE Benchmarks]: http://vlsicad.eecs.umich.edu/BK/Slots/cache/er.cs.ucla.edu/benchmarks/ibm-place/
[UMpack]: http://vlsicad.eecs.umich.edu/BK/PDtools/tar.gz/LATEST/UMpack-45-120708.tar.gz
[UCLApack]: ./UCLApack-43-040113.tar.gz_
[TimberWolf]: https://github.com/rubund/graywolf/blob/master/README.md
