let
  pkgs = import <nixpkgs> { };
in
pkgs.mkShell {
    nativeBuildInputs = with pkgs; [ 
      pkg-config 
    ];

    buildInputs = with pkgs; [
      wayland 
      wayland-scanner
      wayland-protocols
    ];
}
