package My::Suite::TypeMacaddr;
@ISA = qw(My::Suite);
return "Not run for embedded server" if $::opt_embedded_server;
return "No TYPE_MACADDR plugin" unless $ENV{TYPE_MACADDR_SO};
bless { };

