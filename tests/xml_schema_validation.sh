#!/bin/bash
# Validate all test configuration XML files against the variable tree schema.
#
# Files in variableTreeXml/intentionallyBroken/ are excluded on purpose:
# they include files deliberately not conforming to the schema, used to test error handling
# The globs below only cover the top-level files, so the subdirectory is never recursed into.

exit_code=0

for f in *.xml variableTreeXml/*.xml; do
    xmllint --noout --schema ../xmlschema/doocs_variable_tree.xsd "$f" || exit_code=1
done

exit $exit_code
