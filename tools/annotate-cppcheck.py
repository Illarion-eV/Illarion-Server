import xml.etree.ElementTree as ET
import github

annotations = []
errors = 0

root = ET.parse('/tmp/cppcheck.xml').getroot()
for error_tag in root.findall('errors/error'):
    errors = errors + 1

    for location_tag in error_tag.findall('location'):
        annotations.append({
                            'path': location_tag.get('file'),
                            'start_line': int(location_tag.get('line')),
                            'end_line': int(location_tag.get('line')),
                            'annotation_level': 'failure',
                            'message': error_tag.get('verbose'),
                            'title': error_tag.get('msg') + ' [' + error_tag.get('id') + ']'
                          })

github.annotate('Cppcheck Results', errors, annotations)
