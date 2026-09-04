set -a       # Automatically export new variable assignments
. ./.env     # Read assignments from .env
set +a       # Turn automatic exporting off again
./build/jacli  # Inherits the exported variables
