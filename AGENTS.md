# Purpose
Agents are responsible for exploring this repository’s codebase and recording their findings.  
All observations, connections, and insights must be written to `DISCOVERIES.md`.

## Resources
- build.sh will get you the dependencies needed to build es, configure, and make es.
- es-shell.bin is a build of es, you should be able to run it without needing to build, unless there were changes to the source.

## Exploration Guidelines
- Traverse the repository structure systematically.
- Identify key files, directories, and build systems (e.g., `Makefile`, `CMakeLists.txt`, `meson.build`).
- Map out relationships between modules, functions, and data structures.
- Note entry points, core loops, and interfaces (public APIs, CLI commands, network endpoints, etc.).
- Track dependencies: both internal (module-to-module) and external (libraries, packages, OS calls).
- Pay attention to naming conventions, coding patterns, and unusual design choices.

## Logging Guidelines
- All findings must be appended to `DISCOVERIES.md`.
- Use a clear, chronological log format:
  - **Date/Time**
  - **File/Component**
  - **Discovery** (short title)
  - **Details** (one or more paragraphs, with references to code lines/functions if relevant)
- Group related findings under thematic headings (e.g., “Build System”, “Networking Layer”, “CLI Interface”).
- Keep entries factual and technical. Avoid speculation unless explicitly marked as such.

## Workflow
1. Explore a part of the repository.
2. Write a log entry in `DISCOVERIES.md` following the format above.
3. Cross-reference prior discoveries where relevant.
4. Continue until the entire repository has been mapped.

## Example Log Entry

```markdown
### [2025-09-13] Build System – Makefile

**Discovery:** Central build entry point

**Details:**  
Found `Makefile` at the repository root. Defines default target `all` which compiles `src/*.c` into `bin/es-shell`. Links against `readline` and `pthread`. No test targets present. The `clean` target removes only object files, not binaries.
