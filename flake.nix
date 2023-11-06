{
  description = "Image Resizer using libvips";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = "github:numtide/nix-filter";
    pre-commit-hooks.url = "github:cachix/pre-commit-hooks.nix";
    pre-commit-hooks.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    nix-filter,
    pre-commit-hooks,
    ...
  }: let
    pkgsFor = system: import nixpkgs {inherit system;};
    filteredSrc = nix-filter.lib {
      root = ./.;
      include = [
        "src/"
      ];
    };
  in
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = pkgsFor system;
        precommitCheck = pre-commit-hooks.lib.${system}.run {
          src = ./.;
          hooks = {
            actionlint.enable = true;
            hlint.enable = true;
            hpack.enable = true;
            markdownlint.enable = true;
            nil.enable = true;
            alejandra.enable = true;
            ormolu.enable = true;
            clang-format.enable = true;
          };
        };
      in {
        packages.default = pkgs.clangStdenv.mkDerivation {
          name = "vips-linear-resample";
          src = filteredSrc;
          buildInputs = [pkgs.vips pkgs.glibc pkgs.glib pkgs.pkg-config pkgs.llvmPackages.openmp];
          meta = with pkgs.lib; {
            description = "C program to resize images using libvips";
            license = licenses.mit;
          };
          buildPhase = ''
            $CC $(pkg-config vips --cflags --libs) -fopenmp -lpthread -lvips -O3 -o vips-linear-resample src/vips-linear-resample.c
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp vips-linear-resample $out/bin
          '';
        };
        devShells.default = (pkgs.mkShell.override {stdenv = pkgs.clangStdenv;}) {
          packages = with pkgs; [vips glib pkg-config clang-tools];
          shellHook = precommitCheck.shellHook;
        };
      }
    );
  nixConfig = {
    extra-substituters = "https://opensource.cachix.org";
    extra-trusted-public-keys = "opensource.cachix.org-1:6t9YnrHI+t4lUilDKP2sNvmFA9LCKdShfrtwPqj2vKc=";
  };
}
