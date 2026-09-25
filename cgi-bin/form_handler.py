#!/usr/bin/env python3
import cgi
import html

print("Content-Type: text/html\r\n\r\n")

form = cgi.FieldStorage()
name = form.getfirst("name", "Anonymous")
email = form.getfirst("email", "No email")
message = form.getfirst("message", "No message")

print(f"""
<html>
<head><title>Form Result</title></head>
<body>
<h1>Form submission received!</h1>
<p><b>Name:</b> {html.escape(name)}</p>
<p><b>Email:</b> {html.escape(email)}</p>
<p><b>Message:</b> {html.escape(message)}</p>
<a href="/">Back to home</a>
</body>
</html>
""")
