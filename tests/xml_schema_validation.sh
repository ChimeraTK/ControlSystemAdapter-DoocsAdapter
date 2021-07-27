#!/bin/bash 

for f in *.xml variableTreeXml/*.xml; do
    [[ $f = *codeIsNotInt.xml ]] && continue
    xmllint --noout --schema ../xmlschema/doocs_variable_tree.xsd $f
done
