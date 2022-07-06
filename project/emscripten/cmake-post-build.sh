#!/bin/bash

# Copy our 'template.html' into the output folder as 'index.html'.
cp template.html out/index.html

# Navigate into the output folder then start a simple server and open it.
pushd out
#  http-server
python3  -m http.server 1232 & open http://localhost:1232
#    python -m SimpleHTTPServer 8000 & open http://localhost:8000
popd

