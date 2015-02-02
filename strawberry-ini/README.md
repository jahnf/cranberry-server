strawberry-ini 
==============
[![Build Status](https://travis-ci.org/jahnf/strawberry-ini.svg?branch=master)](https://travis-ci.org/jahnf/strawberry-ini)

> Note: The links and references in this markdown file are made
> for github and won't work when shown within the generated
> Doxygen documentation.

A small and lightweight parser for .ini style files written in C.
strawberry-ini has consists of three parts of which most can be used 
seperately if needed:

- [iniParser](include/ini_parser.h): Parse configuration files.
  The main functions are `ini_parse` and `ini_parse_file`.
- [iniDictionary](include/ini_dictionary.h):
  Functions and data structures to manage configuration settings.
- [iniReader](include/ini_reader.h): fills a dictionary from a
  configuration file, uses *iniParser* and *iniDictionary* to do that.
