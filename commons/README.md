# Commons

This directory contains common files used by `kmd` and `lkm`. It adds another layer of abstraction to the project and makes it easier to maintain and add new features.

## Building

This directory does not contain any buildable components. It is used by `kmd` and `lkm` to share common code. The `Makefile` is just for convenience to clean the build intermediates.

## New Features

When considering the addition of a new feature to the project, it's important to first determine its relevance to both `kmd` and `lkm` components. If the feature is applicable to both, it should be integrated into the shared directory. This approach promotes consistency throughout the project, aligning features across both `kmd` and `lkm`.

For features that are incorporated into this shared directory, it's crucial to adhere to the abstractions outlined in the `xplat.h` and `types.h` header files. By doing so, you ensure the new feature's compatibility and applicability with both `kmd` and `lkm`, maintaining the integrity and cohesiveness of the project architecture.