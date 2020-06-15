# Introduction
`srt-live-server` (SLS) is an open source live streaming server for low latency based on Secure Reliable Tranport (SRT).
Normally, the latency of transport by SLS is less than 1 second on the Internet.


# Requirements
Please install the SRT first, refer to SRT (https://github.com/Haivision/srt) for system enviroment.
SLS can only run on OS based on UNIX, such as MacOS, CentOS, Ubuntu, etc.


# Compile
`$ sudo make`

Binary files is generated in the `bin` directory.


# Directivies
About the config file, please see the wiki:
https://github.com/Edward-Wu/srt-live-server/wiki/Directives


# Usage
`$ cd bin`


## 1. Help information
`$ ./sls -h`


## 2. Run with default config file
`$ ./sls -c ../sls.conf`


# Test
SLS only supports the MPEG-TS format streaming.


## 1. Test with FFmpeg
You can push camera live stream by FFmpeg. Please download FFmpeg source code from https://github.com/FFmpeg/FFmpeg, then compile FFmpeg with `--enable-libsrt`.

SRT libraries are installed in folder `/usr/local/lib64`.
If `ERROR: srt >= 1.3.0 not found using pkg-config` occured when compiling FFmpeg, please check the `ffbuild/config.log` file and follow its instruction to resolve this issue. In most cases it can be resolved by the following command:
`export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig`

If `error while loading shared libraries: libsrt.so.1` occured, please add SRT libraries path to file `/etc/ld.so.conf` as the default path, then refresh by comand `/sbin/ldconfig` with root.

Use `ffmpeg` to push camera stream with SRT (on my Mac):

`$ ./ffmpeg -f avfoundation -framerate 30 -i "0:0" -vcodec libx264  -preset ultrafast -tune zerolatency -flags2 local_header  -acodec libmp3lame -g  30 -pkt_size 1316 -flush_packets 0 -f mpegts "srt://[your.sls.ip]:8080?streamid=uplive.sls.com/live/test"`

Play the SRT stream with `ffplay`:

`$ ./ffplay -fflag nobuffer -i "srt://[your.sls.ip]:8080?streamid=live.sls.com/live/test"`


## 2. Test with OBS
The OBS supports SRT protocol to publish stream when version is later than `v25.0`. You can use the following URL:
`srt://[your.sls.ip]:8080?streamid=uplive.sls.com/live/test`
with custom service.


## 3. Test with `srt-live-client`
There is a test tool in `sls`, which can be used for performance testing because of no codec overhead but main network overhead. The `slc` can play a SRT stream to a TS file, or push a TS file to a SRT stream.

Push TS file as SRT URL:

`cd bin`

`$ ./slc -r srt://[your.sls.ip]:8080?streamid=uplive.sls.com/live/test -i [the full file name of exist ts file]`

Play SRT URL:

`$ ./slc -r srt://[your.sls.ip]:8080?streamid=uplive.sls.com/live/test -o [the full file name of ts file to save]`


# Notes:

1. SLS refer to the RTMP URL format (`domain/app/stream_name`). Example: www.sls.com/live/test.
The URL of SLS must be set in streamid parameter of SRT, which will be the unique identification a stream.

2. How to distinguish the publisher and player of the same stream?
In `conf` file, you can set parameters of `domain_player/domain_publisher` and `app_player/app_publisher` to resolve it. Importantly, the two combination strings of `domain_publisher/app_publisher` and `domain_player/app_player` must not be equal in the same server block.

3. I have supplied a simple Android app to test SLS, you can download it from https://github.com/Edward-Wu/liteplayer-srt

# Release notes

## v1.2
1. Update the memory mode, in `v1.1` which is publisher copy data to each player, in `v1.2` each publisher put data to a array and all players read data from this array.
2. Update the relation of the publisher and player, the player is not a member of publisher. The only relation of them is array data.
3. Add push and pull features, support all and hash mode for push, support loop and hash for pull. In cluster mode, you can push a stream to a hash node, and pull this stream from the same hash node.

## v1.2.1
1. Support `hostname:port/app` in upstreams of pull and push.

## v1.3
1. Support reload.
2. Add `idle_streams_timeout` feature for relay.
3. Change license type from GPL to MIT.

## v1.4
1. Add HTTP statistic info.
2. Add HTTP event notification, `on_connect`, `on_close`.
3. Add player feature to `slc` (`srt-live-client`) tool for pressure test.

## v1.4.1
1. Add publisher feature to `slc` (`srt-live-client`) tool, which can push TS file with SRT according DTS.
2. Modify the HTTP bug when host is not available.

## v1.4.2
1. Add `remote_ip` and `remote_port` to `on_event_url` which can be as the unique identification for player or publisher.

## v1.4.3
1. Change the TCP `epoll` mode to select mode for compatible MacOS.
2. Modify the HTTP check repeat bug for reopen.

## v1.4.4
1. OBS streaming compatible, OBS support the SRT protocol which is later than `v25.0`.
(https://obsproject.com/forum/threads/obs-studio-25-0-release-candidate.116067/)

## v1.4.5
1. Add HLS record feature.

## v1.4.6
1. Update the PID file path from `~/` to `/opt/soft/sls/`.
