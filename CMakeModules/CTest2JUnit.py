#!/bin/python

# all credits to https://stackoverflow.com/questions/6329215/how-to-get-ctest-results-in-hudson-jenkins
from lxml import etree
from io import BytesIO
import sys

TAGfile = open(sys.argv[1]+"/Testing/TAG", 'r')
dirname = TAGfile.readline().strip()

xmlfile = open(sys.argv[1]+"/Testing/"+dirname+"/Test.xml", 'r')
xslfile = open(sys.argv[2], 'r')

xslcontent = xslfile.read()

xmldoc = etree.parse(xmlfile)
xslt_root = etree.XML(xslcontent)
transform = etree.XSLT(xslt_root)

result_tree = transform(xmldoc)
print(result_tree)
