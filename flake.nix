{
  description = "Gestor de finanzas en C (CLI/TUI)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        sqlite
        ncurses
      ];
      nativeBuildInputs = with pkgs; [
        gcc
        gnumake
        pkg-config
      ];
		};

		packages.${system}.default = pkgs.stdenv.mkDerivation {
      pname = "finanzas";
      version = "0.1.0";
      
      src = ./.;

      nativeBuildInputs = with pkgs; [ gcc gnumake pkg-config ];
      buildInputs = with pkgs; [ sqlite ncurses ];

      # Nix inyecta la variable $out, que reemplaza al PREFIX de tu Makefile
      installPhase = ''
        make install PREFIX=$out
      '';
    };
  };
}
