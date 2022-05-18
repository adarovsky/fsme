# fsme
A Finite State Machine editor

# installation
To compile FSME, you would need a [Qt](https://www.qt.io) library, 5.15.x (this is what I used, other versions may fit too, as Qt API is very stable).
When downloaded, use either Qt Creator tool or console. To compile from console, run

```sh
qmake
```

This will prepare needed makefiles for your system. Then run

```sh
make && make install
```

You can skip install part and run FSME from where it is built though.

When compiled, open editor and give a new state machine name. Then add one or more states connect them with transitions and save file.

When you have some FSM file, it's time to generate code. Please look into `fsmtools` directory for instructions
