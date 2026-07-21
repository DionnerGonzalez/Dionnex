# Dionnex Kernel Security Policy

## Reporting Security Issues
If you discover a potential security vulnerability in Dionnex kernel core or architectural modules, please email the security team at `security@dionnex.org`.

Do NOT open public GitHub issues for unpatched vulnerabilities.

## Security Architecture
- Kernel Address Space Layout Randomization (KASLR).
- Supervisor Mode Execution Prevention (SMEP) & Supervisor Mode Access Prevention (SMAP).
- Stack Smashing Protector (SSP) with canary checks.
- W^X Page Protection (Pages cannot be simultaneously writable and executable).
