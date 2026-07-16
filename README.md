# UEFI Bootkit with Dynamic Initrd Infection

This project focused on implementing a UEFI-based bootkit. The objective is to achieve early-boot persistence to hide a root-level backdoor. This project demonstrates knowledge of the UEFI environment, the `gnu-efi` toolchain, UEFI function hooking, and operating system-level payload injection.

## Architecture & Application Structure

The bootkit's architecture operates in three distinct phases, each corresponding to an application located in the `srcs/` directory. These applications depend on each other and are built in a specific sequence to embed the malicious payloads securely.

### 1. Initrd Application (`srcs/init/`)
This is the OS-level payload. It contains a custom `run-init` application that establishes a root-level bind shell. To remain stealthy, it launches the original init task isolated within new PID and mount namespaces. This effectively hides the bind shell—running in the original namespaces—from the rest of the OS.
- **Build Output**: It produces an `init` binary which is then packed into a `cpio` archive (`init.cpio`).
- **Embedding**: The `init.cpio` archive is converted into a C header file (`init.h`) containing a byte array using `xxd`.

### 2. UEFI Application (`srcs/uefi/`)
Developed using the `gnu-efi` toolchain, this is the core component. It is a custom UEFI application executed before the primary operating system bootloader. This application hooks into GRUB's `LoadFile2` protocol. When the bootloader uses this protocol to read and load the original `initrd` (initial ramdisk) image, the installed hook intercepts this operation and dynamically appends the `init.cpio` image to it.
- **Dependency**: It includes the `init.h` header generated in the previous step, embedding the cpio archive directly into the UEFI application.
- **Build Output**: It produces the compiled `app.efi` payload.
- **Embedding**: The `app.efi` binary is subsequently converted into a C header file (`uefi.h`) containing a byte array using `xxd`.

### 3. Management Tool (`srcs/rkctl/`)
This is a user-space executable responsible for managing the bootkit from a running system. It provides two main commands: 1) `install` writes the compiled `.efi` payload to the filesystem, and updates the system's UEFI boot order to guarantee that the payload is executed upon reboot. 2) `inspect` allows inspecting the system to check the status of the deployment.
- **Dependency**: It includes the `uefi.h` header, embedding the compiled UEFI bootkit binary into the management tool itself.
- **Build Output**: It produces the final `rkctl` executable. 

*Note: For this project, bypassing Secure Boot is considered out of scope.*

## Build Process (`srcs/Makefile`)

The build process is defined in `srcs/Makefile` and orchestrates a layerd compilation strategy, where each stage embeds the output of the previous stage.
- `make all`: Orchestrates the full build chain, generating the required headers and compiling the applications.
- `make build-*`: Compiles the corresponding application (e.g., `init`, `uefi`, and `rkctl`).
- `make headers`: A utility target that runs `xxd` to generate the `init.h` and `uefi.h` header files containing the C byte arrays for local development.
- `make clean`: Cleans the build directories of all nested projects.

*Note: You can append `DEBUG=1` to any build command (e.g., `make all DEBUG=1`) to enable detailed logging during the execution of the bootkit and payloads.*

By structuring the build this way, the final `rkctl` binary is completely standalone and contains everything needed to deploy the UEFI bootkit and the OS-level backdoor payload.
