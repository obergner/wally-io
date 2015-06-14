# Wallee IO

A fledgling [MQTT 3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) server written in C++11,
using [Boost Asio](http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio.html) for implementing the IO layer.

## Motivation

As of today, this project is mainly aimed at getting familiar with *Boost Asio* and its viability for implementing
custom protocols. 

Moreover this code is part of my ongoing efforts to implement one and the same networked application - I chose an MQTT
server - using different programming languages. There is a [Scala/Akka
Streams](https://github.com/obergner/wallee-io)-based version which is arguably more advanced but still woefully
incomplete.

## Status

This is pretty much pre-alpha software, i.e. it does not even fully implement MQTT's wire protocol, let alone any
serious functionality built on top of that. 
