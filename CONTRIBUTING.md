# Contributing to Dionnex Kernel

Thank you for your interest in contributing to Dionnex!

## Code Style Guidelines
- Dionnex follows the standard Linux Kernel Coding Style.
- Use 8-character tabs for indentation.
- Variable names should be short, concise, and in lowercase (`struct task_struct *task`).
- Function definitions must specify `asmlinkage` or standard visibility modifiers.
- Use `pr_info()`, `pr_warn()`, and `pr_err()` for kernel logging.

## Submitting Patches
1. Ensure your code compiles warning-free with `make W=1`.
2. Format your commit messages clearly: `<subsystem>: brief description`.
3. Sign off on your code (`Signed-off-by: Name <email>`).