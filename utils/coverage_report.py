import xml.dom.minidom

# Read coverage line-rate attribute
doc = xml.dom.minidom.parse('coverage.xml')
cov = doc.getElementsByTagName('coverage')[0]
f = float(cov.getAttribute('line-rate'))

print("COVERAGE PERCENTAGE: %.2f" % (f * 100))
