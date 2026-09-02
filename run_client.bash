set -a       # Automatically export new variable assignments
. ./.env     # Read assignments from .env
set +a       # Turn automatic exporting off again
./my_client  # Inherits the exported variables
