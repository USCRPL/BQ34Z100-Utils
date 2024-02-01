# BQ34Z100-Utils
This project uses the BQ34Z100 driver to implement a Chem ID measurer and an IC debugging/test suite.

## How to set up this project:

1. Clone it to your machine.  Don't forget to use `--recursive` to clone the submodules: `git clone --recursive https://github.com/USCRPL/BQ34Z100G1-Utils.git`
2. Set up the GNU ARM toolchain (and other programs) on your machine using [the toolchain setup guide](https://github.com/mbed-ce/mbed-os/wiki/Toolchain-Setup-Guide).
3. Set up the CMake project for editing.  We have three ways to do this:
    - On the [command line](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-Command-Line)
    - Using the [CLion IDE](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-CLion)
    - Using the [VS Code IDE](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-VS-Code)
4. Edit src/pins.h to configure the pins used for your application
5. Build the `flash-soc-test` or `flash-chem-id-measurer` targets to upload the application to a connected device.

## How to Use the Code
See [here](https://os.mbed.com/users/MultipleMonomials/code/BQ34Z100G1/wiki/Setup-and-Calibration-Guide).