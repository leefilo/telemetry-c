## About the Project

This project started as a way to practice services provided by AWS while also building something tangible end-to-end. The reason I chose the idea of a telemetry sim was because of my interest in cars and the engineering behind them. The app is written in C, exposes a tiny HTTP API, and is deployed to AWS on a single EC2 instance.

As of now, the application will load a set of example sensors and serves them over HTTP with /health and /sensors. It’s small and easy to run so a lot of the focus is on the pipeline and architecture.

This repo is open source and welcome to evolve. 

## What it does now
* C HTTP service with /health and /sensors
* Deployed on EC2; Security Group allows TCP 8080 for testing
* Plain-text responses when using curl to see immediate data

## Future implementations
* Dyanmic reading of YAML files (with focus around car sensors still)

redeploy test: 2025-09-11 09:11:36 UTC
