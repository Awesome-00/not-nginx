# not-nginx

A minimal HTTP/1.1 server written in C from raw sockets — no libraries, no frameworks.

## What it does
- Accepts TCP connections and reads raw HTTP requests
- Parses the request line to extract method and path
- Routes by path, returning a valid HTTP/1.1 response (status line, headers, body)
- Returns 404 for unknown paths

## What it doesn't do (yet)
- Serve real files from disk
- Handle POST or other methods
- Handle multiple simultaneous clients (no fork/threads/select yet)

## Build & run
```bash
gcc httpserver.c -o httpserver
./httpserver
```
Then visit `http://localhost:8080` in a browser.

## Why
Following [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) to understand sockets and HTTP from the ground up rather than starting with a framework.