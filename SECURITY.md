# Security Policy

This library talks to vehicle control units and can clear trouble codes and run manufacturer-specific services. Security reports are therefore taken seriously.

## Supported Versions

Security fixes are applied to the latest code on the default branch and to the most recent release.

## Reporting a Vulnerability

If you find a security issue, **please do not open a public issue**.

Instead, email **muksin.muksin04@gmail.com** with:

- A description of the issue and its potential impact
- Steps to reproduce (board, configuration, sketch)
- A suggested fix, if you have one

You will receive a response as soon as possible, and credit in the release notes if you wish.

## Notes for Users

- The library sends whatever your sketch asks it to send. If your project exposes that over Wi-Fi, Bluetooth or another network, protecting that interface is the job of your project.
- Actuator tests, adaptations and memory access change the state of the vehicle. Use them with the vehicle stationary.
