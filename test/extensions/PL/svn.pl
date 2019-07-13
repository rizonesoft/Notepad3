use strict;
use SVN::Client;
use Git::SVN::Utils;
use Getopt::Long;

my $debug = '';

GetOptions ("debug" => \$debug) 
or die("Error in command line arguments\n");

if (@ARGV < 2) {
    die("Required: command and URL");
}

my $cmd = $ARGV[0];
my $url = $ARGV[1];

# must canonicalise, use Git::SVN::Utils
$url = Git::SVN::Utils::canonicalize_url($url);

if ($debug) {
    print "Command is: $cmd URL is: $url\n";
}

# TODO: deal with auth provider - need a custom one?
my $ctx = new SVN::Client(
              auth => [SVN::Client::get_simple_provider(),
              SVN::Client::get_simple_prompt_provider(\&simple_prompt,2),
              SVN::Client::get_username_provider(),
              SVN::Client::get_ssl_server_trust_prompt_provider(\&cert_prompt)]
              );


if ($cmd eq "info") {
    my $infocallback = sub {
        my( $path, $info, $pool ) = @_;
        if ($debug) {
            print "Current revision of $path is ", $info->rev, "\n";
        }
        
    };

    # NOTE!!! Seems like if you include a slash on the end of this it core dumps in is_canonical
    # Maybe there are methods to standardise
    $ctx->info( $url, undef, 'HEAD', $infocallback, 0 );

    # natural return value is 0, non-zero will be returned if info failed

}
elsif ($cmd eq "ls")
{
    # ls returns a REFERENCE to a hash
    my ($dirents) = $ctx->ls($url, 'HEAD', 0);

    # so we have to dereference the hash with %#
    my $key;
    foreach $key (keys %$dirents) {
        print "$key\n";
    }

}


sub simple_prompt {
    my $cred = shift;
    my $realm = shift;
    my $default_username = shift;
    my $may_save = shift;
    my $pool = shift;

    # Direct prompts to SSH_ASKPASS

    if ($debug) {
        print "Using SourceTree authentication integration\n";
    }

    my $sshaskpass = $ENV{"SSH_ASKPASS"};
    my $username = $ENV{"AUTH_USERNAME"};
    my $hostname = $ENV{"AUTH_HOSTNAME"};

    if (!defined($username) || $username eq "") {
        # Prompt for user name
        my $prompt = "Username for '$hostname':";
        if ($debug) {
            print "Asking SSH_ASKPASS for username with prompt: $prompt\n";
        }
        my $response = qx("$sshaskpass" "$prompt");

        if ($debug) {
            print "SSH_ASKPASS response for username: $response\n";
        }

        if ($? != 0) {
            die("Error calling askpass for username for host $hostname");
        }

        # chop off the newline
        # chomp isn't enough because it doesn't remove windows newlines
        $response =~ s/\r\n//;
        $username = $response;

    }

    if ($debug) {
        print "Asking SSH_ASKPASS for password\n";
    }

    # Fill in username if we retrieved it for password call
    $ENV{"AUTH_USERNAME"} = $username;

    my $prompt = "Password:";
    my $response = qx("$sshaskpass" "$prompt");

    if ($? != 0) {
        die("Error calling askpass for password for $username on $hostname");
    }

    if ($debug) {
        print "SSH_ASKPASS response for password: $response\n";
    }


    $response =~ s/\r\n//;
    my $password = $response;

    if ($debug) {
        print "Passing credentials - username: $username password: $password\n";
    }


    $cred->username($username);
    $cred->password($password);
}

sub cert_prompt {

    # we just trust everything
    my $cred = shift;
    my $realm = shift;
    my $howfailed = shift;
    my $certinfo = shift;

    #print "SSL failure for $realm with value $howfailed, trusting...\n";

    $cred->accepted_failures($howfailed);

}