{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    clang_21
    doxygen
    meson
    ninja
  ];
}
