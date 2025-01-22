# Hello Oracle Injector

This project implements a basic "Hello World" program designed to inject into an Oracle process. It demonstrates the functionality of creating a shared object (SO) file that can be used for process injection.

## Project Structure

```
hello-oracle-injector
├── src
│   ├── aso_dso.c        # Implementation of the injector
│   └── Makefile         # Build instructions for the shared object
└── README.md            # Project documentation
```

## Building the Project

To build the shared object file, navigate to the `src` directory and run the following command:

```bash
make
```

This will compile the `aso_dso.c` file and generate a shared object file named `libhello_oracle_injector.so`.

## Using the Injector

To use the injector with an Oracle process, follow these steps:

1. Ensure that the Oracle process is running.
2. Use the `LD_PRELOAD` environment variable to inject the shared object into the Oracle process. For example:

```bash
LD_PRELOAD=./libhello_oracle_injector.so oracle_process_name
```

3. Upon successful injection, the message "Hello, World!" will be printed to the standard output of the Oracle process.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.