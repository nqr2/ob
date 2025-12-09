{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    pkg-config
    clang_21
    doxygen
    meson
    ninja
  ];
}
