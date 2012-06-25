# The Idea

Use a bound on-disk FIFO. Very much like our Kafka or even our Flume extension the idea is to use a ringbuffer to cope with down times in the ingestion pipeline. It's also what defines our reliability contract. Essentially it's a better and smarted /var/log that one can access via network in a pull based fashion.


## Writing

Every service gets is own individual ringbuffer. The single responsible process (either `bark` or even a `nginx` module linked against the ringbuffer lib) writes to it's space on disk in an append-only fashion (no seeks) with clear defined rules when to flush/sync to disk. Every ringbuffer is bound. Once the configured size has been reached the oldest messages are being pruned.


### Under the Hood

Under the hood the ringbuffer divides the space into segment files. This makes it easier to expire messages.

    > ls /var/orbs/nginx
    -rw-r--r-- 2000000000 1340610000
    -rw-r--r-- 2000000000 1340620100
    -rw-r--r-- 2000000000 1340631000
    -rw-r--r--       1000 1340640101

The above example shows 4 segment files for service/ringbuffer ngix. 3 files have reached capacity. The last one is the one currenly open and being written to. The filename suggests a unix timestamp when the file was created, but by contract is really only an ever increasing number. In combination with the file offset it's a unique pointer at any message inside the ringbuffer ...and therefor is what needs to be stored on the pulling clients as state.

![segments](https://github.com/tcurdt/orbs/raw/master/docs/segments.png)

## Reading

The reading part is completely separate from the writing. A ringserver exposes and interface to read from the ringbuffer over the network. Clients can just connect and request messages for a given service starting at a certain message, effectively resulting in continuous disk reads. The client can provide some very simple filtering rules to ignore unwanted messages. The ringserver basically exposes the following API

 messages, new_timestamp, new_offset = get_messages(service, timestamp, offset, type, type_mask, every)

![architecture](https://github.com/tcurdt/orbs/raw/master/docs/architecture.png)

Ringclients can collect data from all the ringservers. The service/timestamp/offset combination defines the state on the client side.
The type/type_mask/every parameters can be used to tap into the events and create subsets of the data. For example every 10th message could be requested from the ringserver to creating a lower volume sample of the high volume data set. Another option would be to filter just on message type to e.g. extract message classified as "ERROR"s.

## Fault Tolerance

Given the nature of the ringbuffer data of the past is kept on the producing machines. Even if the full pipeline goes down it's all just about bringing the pipeline back in time before messages get expired. Ideally this could mean even a couple of days. This time window should be monitored and the data should be safe in a RAID (mirrored) setup until the pipeline is back online.
If the delay in message delivery is acceptable a distributed failover mechanism could probably be omitted.

