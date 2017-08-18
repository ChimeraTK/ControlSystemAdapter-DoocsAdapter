#!/bin/bash 

for f in *.xml variableTreeXml/*.xml; do
    xmllint --noout --schema ../xmlschema/doocs_variable_tree.xsd $f
done
