local moo = import "moo.jsonnet";
local s = moo.oschema.schema("ez");

local ezs = {
    severity: s.enum("Severity", [
        "Debug", "Log", "Information", "Warning", "Error", "Fatal"
    ]),
                     
    // Fixme: these strings need patterns.
    cname: s.string("ClassName"),
    path: s.string("Path"),
    hostname: s.string("Host"),
    symbol: s.string("Symbol"),
    message: s.string("Message"),

    count: s.number("Count", dtype="i4"),
    ident: s.number("Ident", dtype="u8"),

    context: s.record("Context", [
        s.field("cwd", self.path),
        s.field("file_name", self.path),
        s.field("function_name", self.symbol),
        s.field("host_name", self.hostname),
        s.field("line_number", self.count),
        s.field("package_naem", self.symbol),
        s.field("process_id", self.count),
        s.field("thread_id", self.count),
        // s.field("stack_symbols"), // a void**, go fish
        s.field("stack_size", self.count),
        s.field("user_id", self.count),
        s.field("user_name", self.symbol),
        s.field("application_name", self.symbol),
    ]),
    issue: s.record("Issue", [
        s.field("context", self.context),
        s.field("message", self.message),
        // s.cause("cause", self.issue), // recursion by value?
        s.field("severity", self.severity),
    ]),
};
moo.oschema.sort_select(ezs)

