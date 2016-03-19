import qbs

CppApplication {
    name: "2016-03 Mounting Dojo (CPP-Dresden)"
    targetName: "mounting_dojo"
    consoleApplication: true

    Depends{ name: "cpp" }
    cpp.cxxLanguageVersion: "c++14"
    files: [
        "main.cpp",
        "mounts.cpp",
        "mounts.h",
        "readme.md",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
