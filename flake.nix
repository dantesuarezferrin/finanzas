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
				bear
      ];

      shellHook = ''
        echo "Entorno cargado: gcc, make, pkg-config, sqlite3, ncurses"
      '';
    };
  };
}
