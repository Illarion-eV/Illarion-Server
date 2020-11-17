import github
import re

annotations = []
errors = 0

with open('/tmp/build/tidy.deduplicated', 'r') as tidy_file:
    lines = tidy_file.readlines()
    warning_pattern = re.compile('(src.+?):(\d+):\d+: warning: (.+)')

    for line in lines:
        match = warning_pattern.match(line)

        if match:
            errors = errors + 1

            annotations.append({
                                'path': match.group(1),
                                'start_line': int(match.group(2)),
                                'end_line': int(match.group(2)),
                                'annotation_level': 'failure',
                                'message': '',
                                'title': match.group(3)
                              })

        if annotations[-1]:
            annotations[-1]['message'] += line

github.annotate('Clang-Tidy Results', errors, annotations)
