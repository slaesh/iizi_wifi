import minify_html
import glob


def minifiy(html_content: str) -> str:
    """ minify and sanitize properly """

    print("before", len(html_content))

    minified = minify_html.minify(
        code=html_content,
        minify_js=True,
        minify_css=True,
        remove_processing_instructions=True,
    )

    # take the minified version and make it a one-liner !
    minified = minified.replace("\r\n", "\n")
    lines = minified.split("\n")
    one_liner = ""
    for line in lines:
        trimmed = line.strip()
        if (trimmed.startswith("//")):
            continue

        if (trimmed.startswith("<") and not trimmed.endswith(">")):
            # add space .. imagine multi line html element..
            trimmed += " "

        one_liner += trimmed

    print("after", len(one_liner))

    return one_liner


with open("./html/html.h.template", "r", encoding="utf8") as templateFile:
    template = templateFile.read()

    for htmlFile in glob.glob("./html/*.html"):

        print(htmlFile)
        actual_file_name = htmlFile.split("/")[-1]
        print("actual_file_name", actual_file_name)

        file_content_placeholder = f"__{actual_file_name}__"
        print("file_content_placeholder", file_content_placeholder)

        if (template.find(file_content_placeholder) == -1):
            print("could not find anything to replace..")
            continue

        with open(htmlFile,
                  "r",
                  encoding="utf8"
                  ) as f:
            f_content = minifiy(f.read())
            f_content = f_content.replace("\"", "\\"+"\"")

            template = template.replace(file_content_placeholder, f_content)

    with open("./src/webserver/html/html.h",
              "w",
              encoding="utf8"
              ) as html_header_file:
        html_header_file.write(template)
