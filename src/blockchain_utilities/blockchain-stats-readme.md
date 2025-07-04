# Prolo Blockchain Stats

Prolo Blockchain Stats utlity exports daily statistics for the prolo blockchain from creation through current state.

## Usage:

See also the utility's help option. `prolo-blockchain-stats --help`

From the command line run:

`$ prolo-blockchain-stats`

This loads the existing blockchain and prints the results to the terminal. Default printed data includes Blocks per Day, Total Blocks, Transactions per Day, Total Transactions, Bytes per Day and Total Bytes. The format of the output is in tab delimited csv which is printed to the console. Redirecting or piping the output of the command allows for saving the output to a csv file or feeding your own script accordingly, i.e.:

- `prolo-blockchain-stats > stats-$(date +'%Y-%m-%d').csv`
- `prolo-blockchain-stats | save-to-database.sh`

### Options
`--data-dir arg` 
to specify location of blockchain storage

`--testnet` 
Run on testnet.

`--stagenet`
Run on stagenet.

`--log-level arg`
0-4 or categories

`--block-start arg (=0)`
start at block number

`--block-stop arg (=0)`
Stop at block number

`--with-inputs`
with input stats

`--with-outputs`
with output stats

`--with-ringsize`
with ringsize stats

`--with-hours`
with txns per hour

`--with-emission`
with coin emission

`--with-fees`
with txn fees

`--with-diff`
with difficulty
