# pipeline.es -- demonstrate pipelines with command substitution and status handling

# Capture line count of README via a pipeline
lines = `{cat README.md | wc -l}
echo line-count: $lines

# Pipe standard error to wc using |[2]
errs = `{sh -c 'echo one >&2; echo two >&2; exit 1' |[2] wc -l}
echo stderr-lines: $errs

# Collect exit statuses from a pipeline with a failing command
status = <={echo hi | false}
echo exit-statuses: $status

# Pipe stdout into a custom descriptor and read from it
secret = `{ { echo secret } |[1=3] sh -c 'cat <&3' }
echo custom-fd: $secret

# Nonlinear pipeline comparing outputs via input substitution
same = <={ cmp <{ echo same } <{ echo same } }
echo cmp-same-status: $same
diff = <={ cmp <{ echo left } <{ echo right } >/dev/null }
echo cmp-diff-status: $diff
