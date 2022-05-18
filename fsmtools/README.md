# Finite Machine Editor Code Generator

Code generation script requires [Mako templates](https://www.makotemplates.org) to run. To get prepared, please run the following command:

```sh
pip3 install mako
```
This will install required package for `mkfsm.py` to run.

To generate code from FSME XML file, run the following command

```sh
./mkfsm.py -t fsmts-browser.mako -o MyStateMachine.ts MyStateMachine.fsm
```
An example above will generate Typescript file and example of use at the bottom of it. Go get a full list of options, use

```sh
./mkfsm.py --help
```
