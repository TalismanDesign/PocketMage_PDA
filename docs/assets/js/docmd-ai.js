var DocmdAIAssistant = (() => {
  var __create = Object.create;
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __getProtoOf = Object.getPrototypeOf;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __require = /* @__PURE__ */ ((x) => typeof require !== "undefined" ? require : typeof Proxy !== "undefined" ? new Proxy(x, {
    get: (a, b) => (typeof require !== "undefined" ? require : a)[b]
  }) : x)(function(x) {
    if (typeof require !== "undefined") return require.apply(this, arguments);
    throw Error('Dynamic require of "' + x + '" is not supported');
  });
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };
  var __copyProps = (to, from, except, desc) => {
    if (from && typeof from === "object" || typeof from === "function") {
      for (let key of __getOwnPropNames(from))
        if (!__hasOwnProp.call(to, key) && key !== except)
          __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
    }
    return to;
  };
  var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
    // If the importer is in node compatibility mode or this is not an ESM
    // file that has been converted to a CommonJS file using a Babel-
    // compatible transform (i.e. "__esModule" has not been set), then set
    // "default" to the CommonJS "module.exports" for node compatibility.
    isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
    mod
  ));
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

  // src/client/index.ts
  var index_exports = {};
  __export(index_exports, {
    DocmdAIAssistantUI: () => DocmdAIAssistantUI
  });

  // ../../../node_modules/.pnpm/docmd-assistant@0.1.18/node_modules/docmd-assistant/dist/index.js
  function parseAssistantOutput(raw, knownToolNames) {
    if (!raw || typeof raw !== "string") {
      return { cleanText: "", extractedToolCalls: [] };
    }
    let text = raw;
    const thinkingParts = [];
    const extractedToolCalls = [];
    const thinkingRegex = /<(?:[a-zA-Z0-9_\-]+:)?(think|thought|reasoning|reflection|plan)\b[^>]*>([\s\S]*?)<\/(?:[a-zA-Z0-9_\-]+:)?\1>/gi;
    text = text.replace(thinkingRegex, (_match, _tag, content) => {
      if (content.trim()) thinkingParts.push(content.trim());
      return "";
    });
    const mdThinkingRegex = /```(?:thought|thinking|reasoning|reflection)\s*\n([\s\S]*?)```/gi;
    text = text.replace(mdThinkingRegex, (_match, content) => {
      if (content.trim()) thinkingParts.push(content.trim());
      return "";
    });
    const bracketThinkingRegex = /\[(?:thought|thinking|reasoning):\s*([\s\S]*?)\]/gi;
    text = text.replace(bracketThinkingRegex, (_match, content) => {
      if (content.trim()) thinkingParts.push(content.trim());
      return "";
    });
    text = text.replace(/\]<\][a-zA-Z0-9_\-]+\[>\[[\s\S]*?(?:<\/(?:request|tool_call|action)>|\]<\][a-zA-Z0-9_\-]+\[>\[|$)/gi, "");
    text = text.replace(/\]<\][a-zA-Z0-9_\-]+\[>\[/gi, "");
    text = text.replace(/<\/?(?:[a-zA-Z0-9_\-]+:)?(?:think|thought|reasoning|reflection|plan)\b[^>]*>/gi, "");
    const toolTagRegex = /<(?:[a-zA-Z0-9_\-]+:)?(tool_call|function_call|tool|action|request|invoke)\b([^>]*)>([\s\S]*?)<\/(?:[a-zA-Z0-9_\-]+:)?\1>/gi;
    text = text.replace(toolTagRegex, (match, tag, attrs, body) => {
      const nameMatch = attrs.match(/name=["']([^"']+)["']/i);
      if (nameMatch) {
        const toolName = nameMatch[1].trim();
        if (!knownToolNames || knownToolNames.length === 0 || knownToolNames.includes(toolName)) {
          let args = {};
          try {
            args = JSON.parse(body.trim());
          } catch {
            args = { query: body.trim() };
          }
          extractedToolCalls.push({ name: toolName, args });
          return "";
        }
        return match;
      }
      const initialCount = extractedToolCalls.length;
      tryParseToolJson(body, extractedToolCalls, knownToolNames);
      if (extractedToolCalls.length > initialCount) {
        return "";
      }
      if (tag.toLowerCase() === "tool_call" || tag.toLowerCase() === "function_call") {
        return "";
      }
      return match;
    });
    const inlineFunctionTagRegex = /<function\s*=\s*["']?([a-zA-Z0-9_\-]+)["']?>([\s\S]*?)<\/function>/gi;
    text = text.replace(inlineFunctionTagRegex, (match, toolName, body) => {
      const cleanName = toolName.trim();
      if (!knownToolNames || knownToolNames.length === 0 || knownToolNames.includes(cleanName)) {
        let args = {};
        try {
          args = JSON.parse(body.trim());
        } catch {
          args = { query: body.trim() };
        }
        extractedToolCalls.push({ name: cleanName, args });
        return "";
      }
      return match;
    });
    const mdToolRegex = /```(?:tool_call|function_call|action|json:tool|json)?\s*\n([\s\S]*?)```/gi;
    text = text.replace(mdToolRegex, (_match, body) => {
      const initialCount = extractedToolCalls.length;
      tryParseToolJson(body, extractedToolCalls, knownToolNames);
      if (extractedToolCalls.length > initialCount) {
        return "";
      }
      return _match;
    });
    const bracketToolRegex = /\[(?:TOOL[_\s]?CALL|FUNCTION[_\s]?CALL|TOOL|CALL|ACTION):\s*([a-zA-Z0-9_\-]+)\s*(?:\(([\s\S]*?)\)|(\{[\s\S]*?\})|([\s\S]*?))\]/gi;
    text = text.replace(bracketToolRegex, (match, toolName, parenArgs, braceArgs, plainArgs) => {
      const cleanName = toolName.trim();
      if (!knownToolNames || knownToolNames.length === 0 || knownToolNames.includes(cleanName)) {
        const rawArgs = parenArgs !== void 0 ? parenArgs : braceArgs || plainArgs || "";
        const args = parseFunctionalArgs(rawArgs);
        extractedToolCalls.push({ name: cleanName, args });
        return "";
      }
      return match;
    });
    if (knownToolNames && knownToolNames.length > 0) {
      for (const toolName of knownToolNames) {
        const funcRegex = new RegExp(`(?:call:|invoke:)?\\b${toolName}\\s*\\(([\\s\\S]*?)\\)`, "g");
        text = text.replace(funcRegex, (_match, innerArgs) => {
          let args = {};
          const trimmed = innerArgs.trim();
          if (trimmed.startsWith("{") && trimmed.endsWith("}")) {
            try {
              args = JSON.parse(trimmed);
            } catch {
              args = parseFunctionalArgs(trimmed);
            }
          } else {
            args = parseFunctionalArgs(trimmed);
          }
          extractedToolCalls.push({ name: toolName, args });
          return "";
        });
      }
    }
    text = extractAndStripJsonObjects(text, extractedToolCalls, knownToolNames);
    text = text.replace(/<\/?(?:[a-zA-Z0-9_\-]+:)?(?:tool_call|function_call|invoke)\b[^>]*>/gi, "");
    text = text.replace(/(`{3,}|~{3,})(\w+)(?:[ \t]+|\r?\n)?([\s\S]*?)\1/g, (_match, fence, lang, code) => {
      const trimmedCode = code.replace(/^\s*\n?/, "");
      return fence + lang + "\n" + trimmedCode + fence;
    });
    let cleanText = text.trim();
    const thinking = thinkingParts.length > 0 ? thinkingParts.join("\n\n") : void 0;
    if (!cleanText && thinking) {
      cleanText = thinking;
    }
    return {
      cleanText,
      thinking,
      extractedToolCalls
    };
  }
  function parseFunctionalArgs(argStr) {
    const trimmed = argStr.trim();
    if (!trimmed) return {};
    if (trimmed.startsWith("{") && trimmed.endsWith("}")) {
      try {
        return JSON.parse(trimmed);
      } catch {
      }
    }
    const result = {};
    const paramRegex = /([a-zA-Z0-9_\-]+)\s*=\s*(?:"([^"]*)"|'([^']*)'|(\{[^}]*\})|([a-zA-Z0-9_\-\.]+))/g;
    let match;
    let matchedAny = false;
    while ((match = paramRegex.exec(trimmed)) !== null) {
      matchedAny = true;
      const key = match[1];
      const val = match[2] ?? match[3] ?? match[4] ?? match[5];
      if (val !== void 0) {
        if (val.startsWith("{") && val.endsWith("}")) {
          try {
            result[key] = JSON.parse(val);
          } catch {
            result[key] = val;
          }
        } else if (!isNaN(Number(val))) {
          result[key] = Number(val);
        } else if (val === "true") {
          result[key] = true;
        } else if (val === "false") {
          result[key] = false;
        } else {
          result[key] = val;
        }
      }
    }
    if (!matchedAny && trimmed) {
      result.query = trimmed.replace(/^["']|["']$/g, "");
    }
    return result;
  }
  function tryParseToolJson(rawJson, targetArray, knownToolNames) {
    const trimmed = rawJson.trim();
    if (!trimmed) return false;
    try {
      const parsed = JSON.parse(trimmed);
      return processParsedToolObject(parsed, targetArray, knownToolNames);
    } catch {
      let found = false;
      const matches = findBalancedJsonObjects(trimmed);
      for (const jsonStr of matches) {
        try {
          const parsed = JSON.parse(jsonStr);
          if (processParsedToolObject(parsed, targetArray, knownToolNames)) {
            found = true;
          }
        } catch {
        }
      }
      return found;
    }
  }
  function processParsedToolObject(parsed, targetArray, knownToolNames) {
    if (!parsed || typeof parsed !== "object") return false;
    if (Array.isArray(parsed)) {
      let matched = false;
      for (const item of parsed) {
        if (processParsedToolObject(item, targetArray, knownToolNames)) {
          matched = true;
        }
      }
      return matched;
    }
    if (Array.isArray(parsed.tool_calls) && parsed.tool_calls.length > 0) {
      let matched = false;
      for (const tc of parsed.tool_calls) {
        if (processParsedToolObject(tc, targetArray, knownToolNames)) {
          matched = true;
        }
      }
      return matched;
    }
    if (parsed.function && typeof parsed.function === "object") {
      return processParsedToolObject(parsed.function, targetArray, knownToolNames);
    }
    if (knownToolNames && knownToolNames.length > 0) {
      for (const toolName of knownToolNames) {
        if (parsed[toolName] !== void 0) {
          let toolArgs = parsed[toolName];
          if (typeof toolArgs === "string") {
            try {
              toolArgs = JSON.parse(toolArgs);
            } catch {
              toolArgs = { query: toolArgs };
            }
          } else if (!toolArgs || typeof toolArgs !== "object") {
            toolArgs = {};
          }
          targetArray.push({ name: toolName, args: toolArgs });
          return true;
        }
      }
    }
    const rawName = parsed.name || parsed.tool || parsed.action || parsed.function_name;
    if (rawName && typeof rawName === "string") {
      const toolName = rawName.trim();
      const isKnown = knownToolNames && knownToolNames.length > 0 ? knownToolNames.includes(toolName) : false;
      const hasExplicitToolSignature = parsed.type === "function" || parsed.function_name !== void 0 || parsed.tool !== void 0 || parsed.action_input !== void 0 || parsed.name !== void 0 && (parsed.arguments !== void 0 || parsed.parameters !== void 0 || parsed.input !== void 0);
      if (isKnown || !knownToolNames && hasExplicitToolSignature) {
        let args = parsed.parameters || parsed.arguments || parsed.args || parsed.input || parsed.action_input;
        if (typeof args === "string") {
          try {
            args = JSON.parse(args);
          } catch {
            args = { query: args };
          }
        } else if (!args || typeof args !== "object") {
          args = {};
        }
        targetArray.push({ name: toolName, args });
        return true;
      }
    }
    return false;
  }
  function extractAndStripJsonObjects(text, targetArray, knownToolNames) {
    let result = text;
    const jsonBlocks = findBalancedJsonObjects(text);
    for (const block of jsonBlocks) {
      try {
        const parsed = JSON.parse(block);
        const initialCount = targetArray.length;
        if (processParsedToolObject(parsed, targetArray, knownToolNames)) {
          if (targetArray.length > initialCount) {
            result = result.replace(block, "");
          }
        }
      } catch {
      }
    }
    return result;
  }
  function findBalancedJsonObjects(str) {
    const results = [];
    let depth = 0;
    let startIndex = -1;
    let inString = false;
    let escapeNext = false;
    for (let i = 0; i < str.length; i++) {
      const char = str[i];
      if (escapeNext) {
        escapeNext = false;
        continue;
      }
      if (char === "\\") {
        escapeNext = true;
        continue;
      }
      if (char === '"') {
        inString = !inString;
        continue;
      }
      if (!inString) {
        if (char === "{") {
          if (depth === 0) {
            startIndex = i;
          }
          depth++;
        } else if (char === "}") {
          depth--;
          if (depth === 0 && startIndex !== -1) {
            results.push(str.slice(startIndex, i + 1));
            startIndex = -1;
          }
        }
      }
    }
    return results;
  }
  var ENGINE_VERSION = typeof process !== "undefined" && "0.1.18" ? "0.1.18" : "0.1.17";
  var DEFAULT_SYSTEM_PROMPT = `You are docmd assistant \u2014 a professional, precise, and concise technical AI assistant for this documentation site.

CRITICAL CONSTRAINTS & BEHAVIORAL RULES:
1. IDENTITY: Your name is "docmd assistant". You are an expert AI documentation guide dedicated to assisting visitors with this site. If asked who or what you are, identify yourself as docmd assistant serving this documentation.
2. STRICT SCOPE & BOUNDARIES: Answer strictly about the software, APIs, tools, installation, configuration, and topics documented on this site. Politely decline off-topic queries.
3. STRICT FACTUALITY & ARCHITECTURAL SYNTHESIS:
   - Ground all answers, configuration snippets, and code examples STRICTLY in facts, keys, properties, and evidence explicitly retrieved from documentation search results or site tools.
   - NEVER invent, guess, or fabricate non-existent configurations, non-existent API parameters, or unverified settings.
   - When a user asks about a capability or concept that is not directly available as a single dedicated setting (e.g. pinning an item, custom database auth, etc.), explicitly explain that no dedicated setting exists for that exact query, synthesize the closest supported mechanism or architectural approach (such as explicit navigation configuration via navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to the relevant documentation pages.
   - Never return an empty response, silence, or thought-only output. Always provide an informative, grounded answer to the user.
4. PROFESSIONAL & CONCISE: Provide direct, succinct, and professional answers. Do NOT use excessive emojis. Avoid conversational filler or boilerplate apologies. Get straight to the point.
5. AUTONOMOUS & PROACTIVE TOOL EXECUTION:
   - Always use your tools proactively. Directly execute the appropriate tool (\`search_documentation\`, \`get_site_structure\`, or \`read_documentation_page\`) to retrieve accurate facts before answering.
   - Use \`get_site_structure\` to inspect site topology, available documentation branches, and navigation trees.
   - Use \`search_documentation\` to search release notes, guides, configuration options, and concepts across all projects.
   - If initial search results are inconclusive or return no direct matches, use \`get_site_structure\` or an alternate search before concluding.
   - Use \`read_documentation_page\` when you need full section context or deep code examples.
6. SEARCH STRATEGY \u2014 THIS IS CRITICAL:
   - The search index matches keywords against page titles, headers, and content.
   - Use concise, targeted search keywords.
   - Example: For "how to install or deploy locally", make targeted calls: search("install"), search("deploy").
   - Example: For "what changed in a specific release", search the version or release keyword: search("release notes").
   - Analyze search results carefully, then synthesize your answer.
7. HYPERLINKS & CITATIONS: Always include clickable Markdown hyperlinks \`[Page Title](path)\` in your response for referenced documentation pages.
8. FOUR-BACKTICK CODE FENCES: When providing code blocks, configuration files, or Markdown examples, enclose them in four-backtick fences (\`\`\`\`lang ... \`\`\`\`) rather than three, to prevent fence collision and retain nested code blocks when rendered inside Markdown containers. If the snippet itself contains four backticks, use five backticks.
9. CONCISE & CLEAN OUTPUT: Keep your response clean, structured, and concise. Use valid Markdown formatting without raw unescaped HTML or script tags.`;
  function truncateContextCleanly(text, maxLen = 15e3) {
    if (!text || text.length <= maxLen) return text;
    let sliced = text.slice(0, maxLen);
    const lastDoubleNL = sliced.lastIndexOf("\n\n");
    if (lastDoubleNL > maxLen * 0.5) {
      sliced = sliced.slice(0, lastDoubleNL);
    } else {
      const lastNL = sliced.lastIndexOf("\n");
      if (lastNL > maxLen * 0.5) {
        sliced = sliced.slice(0, lastNL);
      }
    }
    const codeFenceCount = (sliced.match(/(?:`{3,}|~{3,})/g) || []).length;
    if (codeFenceCount % 2 !== 0) {
      sliced += "\n````";
    }
    return sliced + "\n...[context truncated]";
  }
  function getToolStatusInfo(toolName, args) {
    if (toolName === "search_documentation") {
      const query = args?.query || args?.q || "";
      return {
        text: query ? `Searching documentation for "${query}"...` : "Searching documentation...",
        icon: "search"
      };
    }
    if (toolName === "get_site_structure") {
      return {
        text: "Inspecting site navigation & structure...",
        icon: "folder-tree"
      };
    }
    if (toolName === "read_documentation_page") {
      const path = args?.path || args?.url || "";
      return {
        text: path ? `Reading documentation: ${path}...` : "Reading documentation...",
        icon: "book-open"
      };
    }
    if (toolName === "navigate_to_page") {
      const path = args?.path || "";
      return {
        text: path ? `Navigating to ${path}...` : "Navigating to page...",
        icon: "navigation"
      };
    }
    if (toolName === "copy_code_snippet") {
      return {
        text: "Copying code snippet...",
        icon: "copy"
      };
    }
    const readable = toolName.replace(/^([a-z])/, (m) => m.toUpperCase()).replace(/_/g, " ").replace(/([a-z])([A-Z])/g, "$1 $2");
    return {
      text: `${readable}...`,
      icon: "cog"
    };
  }
  var DocmdAssistantEngine = class _DocmdAssistantEngine {
    options;
    history = [];
    tools = /* @__PURE__ */ new Map();
    systemPrompt;
    listeners = /* @__PURE__ */ new Map();
    isExecuting = false;
    contextWindow;
    constructor(options = {}) {
      this.options = { ...options };
      this.systemPrompt = options.systemPrompt || DEFAULT_SYSTEM_PROMPT;
      this.contextWindow = options.contextWindow && options.contextWindow >= 1e3 ? options.contextWindow : 2e5;
      if (options.history) {
        this.history = [...options.history];
      }
      if (options.tools) {
        for (const tool of options.tools) {
          this.registerTool(tool);
        }
      }
    }
    // --- Tool Registration Surface ---
    registerTool(tool) {
      if (!tool.name) {
        throw new Error('Tool must have a valid "name" property.');
      }
      this.tools.set(tool.name, tool);
      return this;
    }
    unregisterTool(name) {
      return this.tools.delete(name);
    }
    getTools() {
      return Array.from(this.tools.values());
    }
    getTool(name) {
      return this.tools.get(name);
    }
    // --- System Prompt & Configuration Surface ---
    setSystemPrompt(prompt) {
      this.systemPrompt = prompt;
      return this;
    }
    appendSystemPrompt(additionalPrompt) {
      this.systemPrompt += `

${additionalPrompt}`;
      return this;
    }
    getSystemPrompt() {
      return this.systemPrompt;
    }
    updateOptions(newOptions) {
      this.options = { ...this.options, ...newOptions };
      if (newOptions.systemPrompt) {
        this.systemPrompt = newOptions.systemPrompt;
      }
      return this;
    }
    // --- State & History Surface ---
    getHistory() {
      return [...this.history];
    }
    setHistory(history) {
      this.history = [...history];
      return this;
    }
    clearHistory() {
      this.history = [];
      this.emit("clear", null);
      return this;
    }
    addMessage(message) {
      this.history.push({
        ...message,
        timestamp: message.timestamp || Date.now()
      });
      return this;
    }
    // --- Event Emitter Surface ---
    on(event, listener) {
      if (!this.listeners.has(event)) {
        this.listeners.set(event, /* @__PURE__ */ new Set());
      }
      this.listeners.get(event).add(listener);
      return this;
    }
    off(event, listener) {
      const set = this.listeners.get(event);
      if (set) {
        set.delete(listener);
      }
      return this;
    }
    emit(type, data) {
      const set = this.listeners.get(type);
      if (set) {
        const event = { type, data };
        for (const listener of set) {
          try {
            listener(event);
          } catch (err) {
            console.error(`[docmd-assistant] Event listener error (${type}):`, err);
          }
        }
      }
    }
    // --- Synchronous Messaging Surface ---
    async sendMessage(content, overrideOptions) {
      if (this.isExecuting) {
        throw new Error("Assistant is currently processing a response. Please wait for the current query to complete.");
      }
      this.isExecuting = true;
      try {
        const userMsg = {
          role: "user",
          content,
          sender: "user",
          timestamp: Date.now()
        };
        this.addMessage(userMsg);
        this.emit("message", userMsg);
        const opts = { ...this.options, ...overrideOptions };
        const endpoint = opts.relayUrl || opts.endpoint;
        if (opts.apiKey || opts.provider === "ollama") {
          return await this.runAiplugLoop(opts);
        }
        return await this.runRelayLoop(endpoint || "https://api.docmd.io/v1/ai/chat", opts);
      } finally {
        this.isExecuting = false;
      }
    }
    // --- Live Streaming Surface ---
    async sendMessageStream(content, callbacks = {}, overrideOptions) {
      if (this.isExecuting) {
        throw new Error("Assistant is currently processing a response. Please wait for the current query to complete.");
      }
      this.isExecuting = true;
      try {
        const userMsg = {
          role: "user",
          content,
          sender: "user",
          timestamp: Date.now()
        };
        this.addMessage(userMsg);
        this.emit("message", userMsg);
        const opts = { ...this.options, ...overrideOptions };
        const endpoint = opts.relayUrl || opts.endpoint;
        callbacks.onStatus?.({ text: "Thinking...", icon: "brain" });
        this.emit("status", { text: "Thinking...", icon: "brain" });
        let response;
        if (opts.apiKey || opts.provider === "ollama") {
          response = await this.runAiplugStreamLoop(opts, callbacks);
        } else {
          response = await this.runRelayStreamLoop(endpoint || "https://api.docmd.io/v1/ai/chat", opts, callbacks);
        }
        callbacks.onFinish?.(response);
        return response;
      } catch (err) {
        callbacks.onError?.(err);
        this.emit("error", err);
        throw err;
      } finally {
        this.isExecuting = false;
      }
    }
    // --- Multi-Turn Autonomous Tool Execution Loop (Direct aiplug Mode) ---
    async runAiplugLoop(opts) {
      const { createLLMAdapter } = await import("aiplug");
      const reasoningVal = opts.reasoning ?? false;
      const adapterOptions = {
        apiKey: opts.apiKey || "",
        baseURL: opts.baseURL,
        ...reasoningVal ? { options: { providerOptions: { reasoning: reasoningVal } } } : {}
      };
      if (opts.provider) adapterOptions.provider = opts.provider;
      if (opts.model) adapterOptions.model = opts.model;
      const adapter = createLLMAdapter(adapterOptions);
      const toolsDef = this.getTools().map((t) => ({
        name: t.name,
        description: t.description,
        parameters: t.parameters || t.schema || { type: "object", properties: {} }
      }));
      const conversationMessages = [
        { role: "system", content: this.systemPrompt }
      ];
      for (const msg of this.history) {
        const role = msg.sender || msg.role;
        conversationMessages.push({
          role: role === "system" ? "system" : role === "assistant" ? "assistant" : "user",
          content: msg.content
        });
      }
      const maxTurns = 5;
      let turnCount = 0;
      let finalReplyText = "";
      while (turnCount < maxTurns) {
        turnCount++;
        const res = await adapter.converse(conversationMessages, toolsDef.length > 0 ? toolsDef : void 0);
        const rawContent = res.message?.content || "";
        const parsed = parseAssistantOutput(rawContent, this.getTools().map((t) => t.name));
        const toolCallsToExecute = [];
        if (res.message?.toolCalls && res.message.toolCalls.length > 0) {
          for (const tc of res.message.toolCalls) {
            if (this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: tc.id || `call_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
                name: tc.name,
                args: tc.input || {}
              });
            }
          }
        } else if (parsed.extractedToolCalls.length > 0) {
          for (const tc of parsed.extractedToolCalls) {
            if (this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: `call_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
                name: tc.name,
                args: tc.args || {}
              });
            }
          }
        }
        if (parsed.cleanText || rawContent) {
          finalReplyText = parsed.cleanText || rawContent;
        }
        if (toolCallsToExecute.length === 0) {
          break;
        }
        conversationMessages.push({
          role: "assistant",
          content: parsed.cleanText || "",
          toolCalls: toolCallsToExecute.map((tc) => ({ id: tc.id, name: tc.name, input: tc.args }))
        });
        const toolResults = [];
        for (const tc of toolCallsToExecute) {
          this.emit("tool_call", { name: tc.name, args: tc.args, callId: tc.id });
          try {
            const result = await this.executeTool(tc.name, tc.args);
            this.emit("tool_result", { name: tc.name, args: tc.args, result, callId: tc.id });
            toolResults.push({
              toolCallId: tc.id,
              name: tc.name,
              content: typeof result === "string" ? result : JSON.stringify(result)
            });
          } catch (err) {
            toolResults.push({
              toolCallId: tc.id,
              name: tc.name,
              content: `Error: ${err.message || String(err)}`,
              isError: true
            });
          }
        }
        conversationMessages.push({
          role: "tool",
          toolResults
        });
      }
      const assistantMsg = {
        role: "assistant",
        content: finalReplyText || "No response generated.",
        sender: "assistant",
        timestamp: Date.now()
      };
      this.addMessage(assistantMsg);
      this.emit("message", assistantMsg);
      return {
        message: assistantMsg.content,
        role: "assistant",
        history: this.getHistory()
      };
    }
    // --- Multi-Turn Autonomous Tool Execution Loop (Direct aiplug Streaming Mode) ---
    async runAiplugStreamLoop(opts, callbacks) {
      const { createLLMAdapter } = await import("aiplug");
      const reasoningVal = opts.reasoning ?? false;
      const adapterOptions = {
        apiKey: opts.apiKey || "",
        baseURL: opts.baseURL,
        ...reasoningVal ? { options: { providerOptions: { reasoning: reasoningVal } } } : {}
      };
      if (opts.provider) adapterOptions.provider = opts.provider;
      if (opts.model) adapterOptions.model = opts.model;
      const adapter = createLLMAdapter(adapterOptions);
      const toolsDef = this.getTools().map((t) => ({
        name: t.name,
        description: t.description,
        parameters: t.parameters || t.schema || { type: "object", properties: {} }
      }));
      const conversationMessages = [
        { role: "system", content: this.systemPrompt }
      ];
      for (const msg of this.history) {
        const role = msg.sender || msg.role;
        conversationMessages.push({
          role: role === "system" ? "system" : role === "assistant" ? "assistant" : "user",
          content: msg.content
        });
      }
      const maxTurns = 6;
      let turnCount = 0;
      let finalAccumulatedText = "";
      while (turnCount < maxTurns) {
        turnCount++;
        let streamBuffer = "";
        let turnDeltaCount = 0;
        const allowToolsThisTurn = turnCount < maxTurns;
        const toolsToPass = allowToolsThisTurn && toolsDef.length > 0 ? toolsDef : void 0;
        const res = await adapter.converseStream(
          conversationMessages,
          toolsToPass,
          (delta) => {
            streamBuffer += delta;
            const isFirst = turnDeltaCount === 0;
            turnDeltaCount++;
            callbacks.onChunk?.(delta, {
              replace: isFirst && turnCount > 1,
              turn: turnCount,
              isFinal: false
            });
            this.emit("chunk", delta);
          }
        );
        const parsed = parseAssistantOutput(streamBuffer, this.getTools().map((t) => t.name));
        const toolCallsToExecute = [];
        if (allowToolsThisTurn && res.message?.toolCalls && res.message.toolCalls.length > 0) {
          for (const tc of res.message.toolCalls) {
            if (this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: tc.id || `call_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
                name: tc.name,
                args: tc.input || {}
              });
            }
          }
        } else if (allowToolsThisTurn && parsed.extractedToolCalls.length > 0) {
          for (const tc of parsed.extractedToolCalls) {
            if (this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: `call_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
                name: tc.name,
                args: tc.args || {}
              });
            }
          }
        }
        if (parsed.cleanText || streamBuffer) {
          finalAccumulatedText = parsed.cleanText || streamBuffer;
        }
        if (toolCallsToExecute.length === 0) {
          callbacks.onChunk?.(finalAccumulatedText, {
            replace: true,
            turn: turnCount,
            isFinal: true
          });
          this.emit("chunk", finalAccumulatedText);
          break;
        }
        conversationMessages.push({
          role: "assistant",
          content: parsed.cleanText || "",
          toolCalls: toolCallsToExecute.map((tc) => ({ id: tc.id, name: tc.name, input: tc.args }))
        });
        const toolResults = [];
        for (const tc of toolCallsToExecute) {
          const statusInfo = getToolStatusInfo(tc.name, tc.args);
          callbacks.onStatus?.(statusInfo);
          this.emit("status", statusInfo);
          callbacks.onToolCall?.({ name: tc.name, args: tc.args, callId: tc.id });
          this.emit("tool_call", { name: tc.name, args: tc.args, callId: tc.id });
          try {
            const result = await this.executeTool(tc.name, tc.args);
            callbacks.onToolResult?.({ name: tc.name, args: tc.args, result, callId: tc.id });
            this.emit("tool_result", { name: tc.name, args: tc.args, result, callId: tc.id });
            toolResults.push({
              toolCallId: tc.id,
              name: tc.name,
              content: typeof result === "string" ? result : JSON.stringify(result)
            });
          } catch (err) {
            toolResults.push({
              toolCallId: tc.id,
              name: tc.name,
              content: `Error: ${err.message || String(err)}`,
              isError: true
            });
          }
        }
        callbacks.onStatus?.({ text: "Thinking...", icon: "brain" });
        this.emit("status", { text: "Thinking...", icon: "brain" });
        conversationMessages.push({
          role: "tool",
          toolResults
        });
      }
      const assistantMsg = {
        role: "assistant",
        content: finalAccumulatedText || "No response generated.",
        sender: "assistant",
        timestamp: Date.now()
      };
      this.addMessage(assistantMsg);
      this.emit("message", assistantMsg);
      return {
        message: assistantMsg.content,
        role: "assistant",
        history: this.getHistory()
      };
    }
    // --- Multi-Turn Autonomous Tool Execution Loop (Relay Mode) ---
    async runRelayLoop(endpoint, opts) {
      const reasoningVal = opts.reasoning ?? false;
      const registeredTools = this.getTools().map((t) => ({
        name: t.name,
        description: t.description,
        parameters: t.parameters || t.schema
      }));
      const rawUserContent = this.history[this.history.length - 1]?.content || "";
      const originalUserQuery = rawUserContent.replace(/\n\n\[Documentation Search Context[\s\S]*$/i, "").trim();
      let currentHistory = this.history.slice(0, -1).map((m) => ({
        sender: m.sender || m.role,
        text: m.content
      }));
      if (currentHistory.length > 12) {
        currentHistory = currentHistory.slice(-12);
      }
      let userMessage = originalUserQuery;
      let allowTools = true;
      const maxTurns = 5;
      let turnCount = 0;
      let finalReply = "";
      const accumulatedToolSummaries = [];
      let lastParsed = null;
      while (turnCount < maxTurns) {
        turnCount++;
        const payload = {
          projectId: opts.projectId,
          siteId: opts.projectId,
          message: userMessage,
          pageUrl: typeof location !== "undefined" ? location.href : void 0,
          pageTitle: typeof document !== "undefined" ? document.title : void 0,
          history: currentHistory,
          systemPrompt: this.systemPrompt,
          reasoning: reasoningVal,
          tools: allowTools && registeredTools.length > 0 ? registeredTools : void 0
        };
        if (turnCount > 1) {
          payload.isToolFollowUp = true;
        }
        if (opts.provider) payload.provider = opts.provider;
        if (opts.model) payload.model = opts.model;
        let res;
        try {
          res = await fetch(endpoint, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "X-Docmd-Plugin": `docmd-assistant/${ENGINE_VERSION}`,
              ...opts.headers || {}
            },
            body: JSON.stringify(payload)
          });
        } catch (fetchErr) {
          const msg = String(fetchErr?.message || fetchErr || "");
          if (fetchErr?.name === "AbortError") {
            throw new Error("Request was aborted or cancelled.");
          }
          if (msg.toLowerCase().includes("load failed") || msg.toLowerCase().includes("failed to fetch")) {
            throw new Error("Network connection to AI relay failed. Please verify your connection or try again.");
          }
          throw fetchErr;
        }
        const data = await res.json();
        if (data.unconfigured) {
          return {
            message: data.message || "Configuration incomplete.",
            role: "assistant",
            unconfigured: true,
            unconfiguredData: data,
            history: this.getHistory()
          };
        }
        if (!res.ok || data.error) {
          throw new Error(data.error || `Relay error (${res.status})`);
        }
        const rawReply = data.text || data.reply || data.response || data.message || "";
        const parsed = parseAssistantOutput(rawReply, registeredTools.map((t) => t.name));
        lastParsed = parsed;
        const toolCallsToExecute = [];
        if (data.tool_calls && Array.isArray(data.tool_calls) && data.tool_calls.length > 0) {
          for (const tc of data.tool_calls) {
            const toolName = tc.name || tc.function?.name;
            const toolArgs = typeof tc.arguments === "string" ? JSON.parse(tc.arguments) : tc.arguments || tc.args || {};
            if (toolName && this.tools.has(toolName)) {
              toolCallsToExecute.push({
                id: tc.id || `call_${Date.now()}`,
                name: toolName,
                args: toolArgs
              });
            }
          }
        } else if (parsed.extractedToolCalls.length > 0) {
          for (const tc of parsed.extractedToolCalls) {
            if (tc.name && this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: `call_${Date.now()}`,
                name: tc.name,
                args: tc.args || {}
              });
            }
          }
        }
        if (parsed.cleanText || rawReply) {
          finalReply = parsed.cleanText || rawReply;
        }
        if (toolCallsToExecute.length === 0) {
          break;
        }
        if (currentHistory.length === 0 || currentHistory[currentHistory.length - 1]?.text !== originalUserQuery) {
          currentHistory.push({
            sender: "user",
            text: originalUserQuery
          });
        }
        for (const tc of toolCallsToExecute) {
          this.emit("tool_call", { name: tc.name, args: tc.args, callId: tc.id });
          const result = await this.executeTool(tc.name, tc.args);
          this.emit("tool_result", { name: tc.name, args: tc.args, result, callId: tc.id });
          const resultStr = typeof result === "string" ? result : JSON.stringify(result, null, 2);
          accumulatedToolSummaries.push(`[Search Result for ${tc.name}]:
${resultStr}`);
          currentHistory.push({
            sender: "assistant",
            text: `[Tool Call: ${tc.name}(${JSON.stringify(tc.args)})]`
          });
          currentHistory.push({
            sender: "user",
            text: `[Tool Result for ${tc.name}]: ${resultStr.length > 2e3 ? resultStr.slice(0, 2e3) + "..." : resultStr}`
          });
        }
        const contextStr = truncateContextCleanly(accumulatedToolSummaries.join("\n\n"), this.contextWindow);
        userMessage = `User Question: ${originalUserQuery}

Retrieved Documentation Context:
${contextStr}

Based on the retrieved documentation context above and your knowledge of docmd architecture, answer the user's question directly with concise explanations, exact commands, and clickable Markdown links. If the documentation does not have an explicit, dedicated configuration option or feature matching the user's exact query, clearly state this, synthesize the closest supported mechanism or architectural pattern (such as explicit navigation configuration in navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to the relevant documentation pages. Never return an empty response. Do not repeat introductory greetings.`;
        if (turnCount >= 2) {
          allowTools = false;
        }
      }
      if (!finalReply || !finalReply.trim()) {
        if (lastParsed?.thinking && lastParsed.thinking.trim().length > 20) {
          finalReply = lastParsed.thinking.trim();
        } else {
          finalReply = `No documentation page in the retrieved search results explicitly covers a dedicated setting for "${originalUserQuery}". You can configure or customize this behavior via site navigation structure or [Project Structure](https://docs.docmd.io/getting-started/project-structure/).`;
        }
      }
      const assistantMsg = {
        role: "assistant",
        content: finalReply,
        sender: "assistant",
        timestamp: Date.now()
      };
      this.addMessage(assistantMsg);
      this.emit("message", assistantMsg);
      return {
        message: finalReply,
        role: "assistant",
        history: this.getHistory()
      };
    }
    // --- Multi-Turn Autonomous Tool Execution Loop (Relay Streaming Mode) ---
    async runRelayStreamLoop(endpoint, opts, callbacks) {
      const reasoningVal = opts.reasoning ?? false;
      const registeredTools = this.getTools().map((t) => ({
        name: t.name,
        description: t.description,
        parameters: t.parameters || t.schema
      }));
      const rawUserContent = this.history[this.history.length - 1]?.content || "";
      const originalUserQuery = rawUserContent.replace(/\n\n\[Documentation Search Context[\s\S]*$/i, "").trim();
      let currentHistory = this.history.slice(0, -1).map((m) => ({
        sender: m.sender || m.role,
        text: m.content
      }));
      if (currentHistory.length > 12) {
        currentHistory = currentHistory.slice(-12);
      }
      let userMessage = originalUserQuery;
      let allowTools = true;
      const maxTurns = 6;
      let turnCount = 0;
      let finalReply = "";
      const accumulatedToolSummaries = [];
      let lastParsed = null;
      while (turnCount < maxTurns) {
        turnCount++;
        let turnDeltaCount = 0;
        const payload = {
          projectId: opts.projectId,
          siteId: opts.projectId,
          message: userMessage,
          pageUrl: typeof location !== "undefined" ? location.href : void 0,
          pageTitle: typeof document !== "undefined" ? document.title : void 0,
          history: currentHistory,
          systemPrompt: this.systemPrompt,
          reasoning: reasoningVal,
          tools: allowTools && registeredTools.length > 0 ? registeredTools : void 0,
          stream: true
        };
        if (turnCount > 1) {
          payload.isToolFollowUp = true;
        }
        if (opts.provider) payload.provider = opts.provider;
        if (opts.model) payload.model = opts.model;
        let res;
        try {
          res = await fetch(endpoint, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "Accept": "text/event-stream, application/json",
              "X-Docmd-Plugin": `docmd-assistant/${ENGINE_VERSION}`,
              ...opts.headers || {}
            },
            body: JSON.stringify(payload)
          });
        } catch (fetchErr) {
          const msg = String(fetchErr?.message || fetchErr || "");
          if (fetchErr?.name === "AbortError") {
            throw new Error("Request was aborted or cancelled.");
          }
          if (msg.toLowerCase().includes("load failed") || msg.toLowerCase().includes("failed to fetch")) {
            throw new Error("Network connection to AI relay failed. Please verify your connection or try again.");
          }
          throw fetchErr;
        }
        const contentType = res.headers.get("content-type") || "";
        const isEventStream = contentType.toLowerCase().includes("text/event-stream");
        if (!isEventStream || !res.body) {
          const textData = await res.text();
          let data;
          try {
            data = JSON.parse(textData);
          } catch {
            data = { text: textData };
          }
          if (data.unconfigured) {
            return {
              message: data.message || "Configuration incomplete.",
              role: "assistant",
              unconfigured: true,
              unconfiguredData: data,
              history: this.getHistory()
            };
          }
          if (!res.ok || data.error) {
            throw new Error(data.error || `Relay error (${res.status})`);
          }
          const rawReply = data.text || data.reply || data.response || data.message || "";
          const parsed2 = parseAssistantOutput(rawReply, registeredTools.map((t) => t.name));
          lastParsed = parsed2;
          const toolCallsToExecute2 = [];
          if (data.tool_calls && Array.isArray(data.tool_calls) && data.tool_calls.length > 0) {
            for (const tc of data.tool_calls) {
              toolCallsToExecute2.push({
                id: tc.id || `call_${Date.now()}`,
                name: tc.name || tc.function?.name,
                args: typeof tc.arguments === "string" ? JSON.parse(tc.arguments) : tc.arguments || tc.args || {}
              });
            }
          } else if (parsed2.extractedToolCalls.length > 0) {
            for (const tc of parsed2.extractedToolCalls) {
              toolCallsToExecute2.push({
                id: `call_${Date.now()}`,
                name: tc.name,
                args: tc.args || {}
              });
            }
          }
          if (toolCallsToExecute2.length === 0) {
            finalReply = parsed2.cleanText || rawReply || "No response returned.";
            callbacks.onChunk?.(finalReply, { replace: turnCount > 1, turn: turnCount, isFinal: true });
            this.emit("chunk", finalReply);
            break;
          }
          if (currentHistory.length === 0 || currentHistory[currentHistory.length - 1]?.text !== originalUserQuery) {
            currentHistory.push({
              sender: "user",
              text: originalUserQuery
            });
          }
          for (const tc of toolCallsToExecute2) {
            const statusInfo = getToolStatusInfo(tc.name, tc.args);
            callbacks.onStatus?.(statusInfo);
            this.emit("status", statusInfo);
            callbacks.onToolCall?.({ name: tc.name, args: tc.args, callId: tc.id });
            this.emit("tool_call", { name: tc.name, args: tc.args, callId: tc.id });
            const result = await this.executeTool(tc.name, tc.args);
            callbacks.onToolResult?.({ name: tc.name, args: tc.args, result, callId: tc.id });
            this.emit("tool_result", { name: tc.name, args: tc.args, result, callId: tc.id });
            const resultStr = typeof result === "string" ? result : JSON.stringify(result, null, 2);
            accumulatedToolSummaries.push(`[Search Result for ${tc.name}]:
${resultStr}`);
            currentHistory.push({
              sender: "assistant",
              text: `[Tool Call: ${tc.name}(${JSON.stringify(tc.args)})]`
            });
            currentHistory.push({
              sender: "user",
              text: `[Tool Result for ${tc.name}]: ${resultStr.length > 2e3 ? resultStr.slice(0, 2e3) + "..." : resultStr}`
            });
          }
          const contextStr2 = truncateContextCleanly(accumulatedToolSummaries.join("\n\n"), this.contextWindow);
          userMessage = `User Question: ${originalUserQuery}

Retrieved Documentation Context:
${contextStr2}

Based on the retrieved documentation context above and your knowledge of docmd architecture, answer the user's question directly with concise explanations, exact commands, and clickable Markdown links. If the documentation does not have an explicit, dedicated configuration option or feature matching the user's exact query, clearly state this, synthesize the closest supported mechanism or architectural pattern (such as explicit navigation configuration in navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to the relevant documentation pages. Never return an empty response. Do not repeat introductory greetings.${turnCount >= maxTurns - 1 ? "\n\nYou must now synthesize your complete, final response based on all information retrieved. Do not call any further tools." : ""}`;
          if (turnCount >= maxTurns - 1) {
            allowTools = false;
          }
          continue;
        }
        const reader = res.body.getReader();
        const decoder = new TextDecoder("utf-8");
        let buffer = "";
        let streamReplyText = "";
        const sseToolCalls = [];
        while (true) {
          const { done, value } = await reader.read();
          if (done) break;
          buffer += decoder.decode(value, { stream: true });
          const lines = buffer.split(/\r?\n/);
          buffer = lines.pop() || "";
          for (const line of lines) {
            const trimmed = line.trim();
            if (!trimmed || trimmed.startsWith(":")) continue;
            if (trimmed.startsWith("data:")) {
              const dataStr = trimmed.slice(5).trim();
              if (dataStr === "[DONE]") continue;
              try {
                const dataObj = JSON.parse(dataStr);
                if (dataObj.status) {
                  callbacks.onStatus?.(dataObj.status);
                  this.emit("status", dataObj.status);
                }
                if (dataObj.tool_calls && Array.isArray(dataObj.tool_calls)) {
                  sseToolCalls.push(...dataObj.tool_calls);
                }
                if (dataObj.delta) {
                  streamReplyText += dataObj.delta;
                  const isFirst = turnDeltaCount === 0;
                  turnDeltaCount++;
                  callbacks.onChunk?.(dataObj.delta, {
                    replace: isFirst && turnCount > 1,
                    turn: turnCount,
                    isFinal: false
                  });
                  this.emit("chunk", dataObj.delta);
                }
                if (dataObj.text) {
                  streamReplyText = dataObj.text;
                  callbacks.onChunk?.(dataObj.text, {
                    replace: true,
                    turn: turnCount,
                    isFinal: false
                  });
                  this.emit("chunk", dataObj.text);
                }
              } catch {
                streamReplyText += dataStr;
                const isFirst = turnDeltaCount === 0;
                turnDeltaCount++;
                callbacks.onChunk?.(dataStr, {
                  replace: isFirst && turnCount > 1,
                  turn: turnCount,
                  isFinal: false
                });
                this.emit("chunk", dataStr);
              }
            }
          }
        }
        if (!streamReplyText && buffer.trim()) {
          try {
            const parsedBuf = JSON.parse(buffer.trim());
            streamReplyText = parsedBuf.text || parsedBuf.reply || parsedBuf.message || buffer.trim();
            if (parsedBuf.tool_calls && Array.isArray(parsedBuf.tool_calls)) {
              sseToolCalls.push(...parsedBuf.tool_calls);
            }
          } catch {
            streamReplyText = buffer.trim();
          }
        }
        const parsed = parseAssistantOutput(streamReplyText, registeredTools.map((t) => t.name));
        lastParsed = parsed;
        const toolCallsToExecute = [];
        if (sseToolCalls.length > 0) {
          for (const tc of sseToolCalls) {
            const toolName = tc.name || tc.function?.name;
            const toolArgs = typeof tc.arguments === "string" ? JSON.parse(tc.arguments) : tc.arguments || tc.args || {};
            if (toolName && this.tools.has(toolName)) {
              toolCallsToExecute.push({
                id: tc.id || `call_${Date.now()}`,
                name: toolName,
                args: toolArgs
              });
            }
          }
        } else if (parsed.extractedToolCalls.length > 0) {
          for (const tc of parsed.extractedToolCalls) {
            if (tc.name && this.tools.has(tc.name)) {
              toolCallsToExecute.push({
                id: `call_${Date.now()}`,
                name: tc.name,
                args: tc.args || {}
              });
            }
          }
        }
        if (parsed.cleanText || streamReplyText) {
          finalReply = parsed.cleanText || streamReplyText;
        }
        if (toolCallsToExecute.length === 0) {
          callbacks.onChunk?.(finalReply, {
            replace: true,
            turn: turnCount,
            isFinal: true
          });
          this.emit("chunk", finalReply);
          break;
        }
        if (currentHistory.length === 0 || currentHistory[currentHistory.length - 1]?.text !== originalUserQuery) {
          currentHistory.push({
            sender: "user",
            text: originalUserQuery
          });
        }
        for (const tc of toolCallsToExecute) {
          const statusInfo = getToolStatusInfo(tc.name, tc.args);
          callbacks.onStatus?.(statusInfo);
          this.emit("status", statusInfo);
          callbacks.onToolCall?.({ name: tc.name, args: tc.args, callId: tc.id });
          this.emit("tool_call", { name: tc.name, args: tc.args, callId: tc.id });
          const result = await this.executeTool(tc.name, tc.args);
          callbacks.onToolResult?.({ name: tc.name, args: tc.args, result, callId: tc.id });
          this.emit("tool_result", { name: tc.name, args: tc.args, result, callId: tc.id });
          const resultStr = typeof result === "string" ? result : JSON.stringify(result, null, 2);
          accumulatedToolSummaries.push(`[Search Result for ${tc.name}]:
${resultStr}`);
          currentHistory.push({
            sender: "assistant",
            text: `[Tool Call: ${tc.name}(${JSON.stringify(tc.args)})]`
          });
          currentHistory.push({
            sender: "user",
            text: `[Tool Result for ${tc.name}]: ${resultStr.length > 2e3 ? resultStr.slice(0, 2e3) + "..." : resultStr}`
          });
        }
        const contextStr = truncateContextCleanly(accumulatedToolSummaries.join("\n\n"), this.contextWindow);
        userMessage = `User Question: ${originalUserQuery}

Retrieved Documentation Context:
${contextStr}

Based on the retrieved documentation context above and your knowledge of docmd architecture, answer the user's question directly with concise explanations, exact commands, and clickable Markdown links. If the documentation does not have an explicit, dedicated configuration option or feature matching the user's exact query, clearly state this, synthesize the closest supported mechanism or architectural pattern (such as explicit navigation configuration in navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to the relevant documentation pages. Never return an empty response. Do not repeat introductory greetings.${turnCount >= maxTurns - 1 ? "\n\nYou must now synthesize your complete, final response based on all information retrieved. Do not call any further tools." : ""}`;
        if (turnCount >= maxTurns - 1) {
          allowTools = false;
        }
        continue;
      }
      if (!finalReply || !finalReply.trim()) {
        if (lastParsed?.thinking && lastParsed.thinking.trim().length > 20) {
          finalReply = lastParsed.thinking.trim();
        } else if (accumulatedToolSummaries.length > 0) {
          finalReply = `Based on the search results in the documentation, no dedicated setting explicitly matches "${originalUserQuery}". You can configure structure and navigation through [Project Structure](https://docs.docmd.io/getting-started/project-structure/) or your site configuration.`;
        } else {
          finalReply = `No documentation page in the retrieved search results explicitly covers a dedicated setting for "${originalUserQuery}". You can configure or customize this behavior via site navigation structure or [Project Structure](https://docs.docmd.io/getting-started/project-structure/).`;
        }
        callbacks.onChunk?.(finalReply, { replace: true, turn: turnCount, isFinal: true });
        this.emit("chunk", finalReply);
      }
      const assistantMsg = {
        role: "assistant",
        content: finalReply || "No response returned.",
        sender: "assistant",
        timestamp: Date.now()
      };
      this.addMessage(assistantMsg);
      this.emit("message", assistantMsg);
      return {
        message: finalReply,
        role: "assistant",
        history: this.getHistory()
      };
    }
    // --- Tool Execution Pipeline ---
    // Stopwords to strip from multi-word search queries before splitting into individual keywords
    static SEARCH_STOPWORDS = /* @__PURE__ */ new Set([
      "a",
      "an",
      "the",
      "is",
      "are",
      "was",
      "were",
      "be",
      "been",
      "being",
      "have",
      "has",
      "had",
      "do",
      "does",
      "did",
      "will",
      "would",
      "shall",
      "should",
      "may",
      "might",
      "must",
      "can",
      "could",
      "to",
      "of",
      "in",
      "for",
      "on",
      "with",
      "at",
      "by",
      "from",
      "as",
      "into",
      "through",
      "during",
      "before",
      "after",
      "above",
      "below",
      "between",
      "out",
      "up",
      "down",
      "off",
      "over",
      "under",
      "again",
      "further",
      "then",
      "once",
      "here",
      "there",
      "when",
      "where",
      "why",
      "how",
      "all",
      "each",
      "every",
      "both",
      "few",
      "more",
      "most",
      "other",
      "some",
      "such",
      "no",
      "nor",
      "not",
      "only",
      "own",
      "same",
      "so",
      "than",
      "too",
      "very",
      "just",
      "because",
      "but",
      "and",
      "or",
      "if",
      "while",
      "about",
      "what",
      "which",
      "who",
      "whom",
      "this",
      "that",
      "these",
      "those",
      "am",
      "it",
      "its",
      "i",
      "me",
      "my",
      "we",
      "our",
      "you",
      "your",
      "he",
      "she",
      "they",
      "them",
      "his",
      "her",
      "their"
    ]);
    async executeTool(name, rawArgs) {
      const tool = this.tools.get(name);
      if (!tool) {
        return { error: `Tool "${name}" is not registered on this assistant engine.` };
      }
      const handler = tool.handler || tool.execute;
      if (!handler) {
        return { error: `Tool "${name}" has no valid execution handler.` };
      }
      let args = rawArgs;
      if (typeof rawArgs === "string") {
        try {
          args = JSON.parse(rawArgs);
        } catch {
          args = { query: rawArgs, path: rawArgs, code: rawArgs };
        }
      }
      if (!args || typeof args !== "object") {
        args = {};
      }
      if (!args.query && (args.q || args.search_query || args.text || args.input || args.keyword || args.keywords)) {
        args.query = args.q || args.search_query || args.text || args.input || args.keyword || args.keywords;
      }
      if (name === "search_documentation" && args.query && typeof args.query === "string") {
        const rawQuery = args.query.trim();
        const words = rawQuery.toLowerCase().replace(/[^\w\s-]/g, "").split(/\s+/).filter(Boolean);
        const keywords = words.filter((w) => w.length > 2 && !_DocmdAssistantEngine.SEARCH_STOPWORDS.has(w));
        if (keywords.length >= 3) {
          const allResults = [];
          const seenPaths = /* @__PURE__ */ new Set();
          for (const keyword of keywords.slice(0, 5)) {
            try {
              const result = await handler({ ...args, query: keyword }, { engine: this });
              if (Array.isArray(result)) {
                for (const item of result) {
                  const key = item.path || item.url || item.title || JSON.stringify(item);
                  if (!seenPaths.has(key)) {
                    seenPaths.add(key);
                    allResults.push(item);
                  }
                }
              }
            } catch {
            }
          }
          return allResults.length > 0 ? allResults.slice(0, 8) : [];
        }
      }
      try {
        const result = await handler(args, { engine: this });
        return result;
      } catch (err) {
        console.warn(`[docmd-assistant] Tool execution failed for "${name}":`, err);
        this.emit("error", { tool: name, error: err });
        return { error: `Tool execution failed: ${err.message || String(err)}` };
      }
    }
  };
  function extractStructuredContent(mainContent) {
    const clone = mainContent.cloneNode(true);
    const removeSelectors = 'script, style, noscript, nav, aside, footer, header, svg, .sidebar, .toc, .docmd-ai-drawer, .docmd-ai-bar, [aria-hidden="true"]';
    clone.querySelectorAll(removeSelectors).forEach((el) => el.remove());
    clone.querySelectorAll("pre").forEach((pre) => {
      const code = pre.querySelector("code");
      const lang = code?.className?.match(/language-([a-z0-9_-]+)/i)?.[1] || "";
      const codeText = (code || pre).textContent || "";
      const marker = document.createElement("div");
      marker.textContent = `

\`\`\`${lang}
${codeText.trim()}
\`\`\`

`;
      pre.replaceWith(marker);
    });
    for (let lvl = 1; lvl <= 6; lvl++) {
      const hashes = "#".repeat(lvl);
      clone.querySelectorAll(`h${lvl}`).forEach((h) => {
        const t = (h.textContent || "").trim();
        if (t) {
          const div = document.createElement("div");
          div.textContent = `

${hashes} ${t}

`;
          h.replaceWith(div);
        }
      });
    }
    clone.querySelectorAll("li").forEach((li) => {
      const t = (li.textContent || "").trim();
      if (t) {
        const div = document.createElement("div");
        div.textContent = `
- ${t}`;
        li.replaceWith(div);
      }
    });
    clone.querySelectorAll("p").forEach((p) => {
      const t = (p.textContent || "").trim();
      if (t) {
        const div = document.createElement("div");
        div.textContent = `

${t}

`;
        p.replaceWith(div);
      }
    });
    const raw = clone.textContent || "";
    return raw.replace(/\n{3,}/g, "\n\n").trim();
  }
  function createStandardTools(customSearch, customReader) {
    return [
      {
        name: "search_documentation",
        description: "Search documentation content for answers to specific user questions. Optionally filter by version or project.",
        parameters: {
          type: "object",
          properties: {
            query: { type: "string", description: "The search query string" },
            version: { type: "string", description: 'Optional documentation version filter (e.g. "0.9.0", "0.8.0", "latest")' },
            project: { type: "string", description: 'Optional workspace project name or prefix filter (e.g. "/", "assistant", "search")' }
          },
          required: ["query"]
        },
        execute: async (rawArgs) => {
          const query = typeof rawArgs === "string" ? rawArgs : rawArgs?.query || rawArgs?.q || rawArgs?.search_query || rawArgs?.text || rawArgs?.input || "";
          const project = typeof rawArgs === "object" ? rawArgs?.project || rawArgs?.projectFilter : void 0;
          const version = typeof rawArgs === "object" ? rawArgs?.version || rawArgs?.versionFilter : void 0;
          if (customSearch) {
            try {
              return await customSearch(query, project, version);
            } catch (err) {
              console.warn("[docmd-assistant] Custom search failed:", err);
            }
          }
          if (typeof document !== "undefined") {
            const results = [];
            const headings = Array.from(document.querySelectorAll("h1, h2, h3, h4, section"));
            const cleanQuery = (query || "").toLowerCase().trim();
            if (!cleanQuery) return [];
            for (const el of headings) {
              const text = (el.textContent || "").trim();
              if (text.toLowerCase().includes(cleanQuery)) {
                const parent = el.closest("section, article") || el.parentElement;
                const snippet = parent ? parent.textContent?.slice(0, 200) || text : text;
                results.push({
                  title: text,
                  path: window.location.pathname + (el.id ? `#${el.id}` : ""),
                  snippet
                });
              }
            }
            return results.slice(0, 5);
          }
          return [];
        }
      },
      {
        name: "navigate_to_page",
        description: "Navigate user browser to a specific URL or section anchor on the documentation site.",
        parameters: {
          type: "object",
          properties: {
            path: { type: "string", description: "Relative path or anchor hash (e.g. /docs/setup#install)" }
          },
          required: ["path"]
        },
        execute: async ({ path }) => {
          if (typeof window !== "undefined" && path) {
            if (path.startsWith("#")) {
              const target = document.querySelector(path);
              if (target) {
                target.scrollIntoView({ behavior: "smooth" });
                return { success: true, navigatedTo: path };
              }
            }
            window.location.href = path;
            return { success: true, navigatedTo: path };
          }
          return { success: false, reason: "Window object unavailable" };
        }
      },
      {
        name: "copy_code_snippet",
        description: "Copy a code snippet directly to the user clipboard.",
        parameters: {
          type: "object",
          properties: {
            code: { type: "string", description: "The exact code snippet to copy" }
          },
          required: ["code"]
        },
        execute: async ({ code }) => {
          if (typeof navigator !== "undefined" && navigator.clipboard) {
            await navigator.clipboard.writeText(code);
            return { success: true, copiedLength: code.length };
          }
          return { success: false, reason: "Clipboard API unavailable" };
        }
      },
      {
        name: "read_documentation_page",
        description: "Fetch and read the full content of a specific documentation page or section path.",
        parameters: {
          type: "object",
          properties: {
            path: { type: "string", description: "The page path to fetch and read (e.g. /quickstart or /docs/setup)" }
          },
          required: ["path"]
        },
        execute: async ({ path: pagePath }) => {
          if (customReader) {
            try {
              const res = await customReader(pagePath);
              if (typeof res === "string") {
                return { path: pagePath, content: res };
              }
              return { path: pagePath, title: res.title, content: res.content || "" };
            } catch (err) {
              console.warn("[docmd-assistant] Custom reader failed:", err);
            }
          }
          if (typeof window === "undefined") {
            return { error: "Window context unavailable" };
          }
          try {
            const targetUrl = pagePath.startsWith("http") ? pagePath : window.location.origin + (pagePath.startsWith("/") ? pagePath : "/" + pagePath);
            const res = await fetch(targetUrl);
            if (res.ok) {
              const html = await res.text();
              const parser = new DOMParser();
              const doc = parser.parseFromString(html, "text/html");
              const mainContent = doc.querySelector('main, article, [role="main"], body');
              const text = mainContent ? extractStructuredContent(mainContent) : "";
              return {
                path: pagePath,
                content: text ? text.length > 8e3 ? text.slice(0, 8e3) + "\n...[content capped for token economy]" : text : "Page content could not be extracted."
              };
            }
          } catch (err) {
            console.warn("[docmd-assistant] Failed to fetch page content:", err);
          }
          return { error: `Could not load page content for ${pagePath}` };
        }
      }
    ];
  }

  // src/client/index.ts
  var DocmdAIAssistantUI = class {
    constructor() {
      this.engine = null;
      this.container = null;
      this.isDrawerOpened = false;
      this.isPending = false;
      this.projectId = "";
      this.isUnconfigured = false;
      const rawCfg = window.__docmd_ai_config || window.__DOCMD_AI_CONFIG__;
      if (!rawCfg || rawCfg.enabled === false || rawCfg.assistant === false || rawCfg.chat === false) {
        return;
      }
      const cfg = rawCfg;
      this.projectId = cfg.projectId || cfg.siteId || cfg.cloud?.projectId || cfg.cloud?.siteId || "default";
      this.isUnconfigured = (!cfg.projectId || cfg.projectId === "default") && !cfg.apiKey && !cfg.baseURL;
      const initialSystemPrompt = this.buildSystemPrompt();
      this.engine = new DocmdAssistantEngine({
        projectId: this.projectId,
        endpoint: cfg.endpoint || (this.projectId ? "https://api.docmd.io/v1/ai/chat" : void 0),
        provider: cfg.provider,
        model: cfg.model,
        systemPrompt: initialSystemPrompt,
        reasoning: cfg.reasoning ?? false,
        contextWindow: cfg.contextWindow
      });
      const isSemanticUsable = cfg.searchCapabilities?.semantic === true;
      const standardTools = createStandardTools(
        async (query) => {
          return await this.searchAllWorkspaceIndexes(query);
        }
      );
      for (const tool of standardTools) {
        this.engine.registerTool(tool);
      }
      this.engine.registerTool({
        name: "get_site_structure",
        description: "Get the complete documentation site structure, including available versions (current and historical), supported languages/locales, workspace projects, search capabilities, and page navigation hierarchy with titles and URLs.",
        parameters: {
          type: "object",
          properties: {}
        },
        execute: async () => {
          return this.getSiteStructure();
        }
      });
      this.engine.registerTool({
        name: "search_documentation",
        description: `Search documentation pages across all projects in this workspace using full-text keyword matching ${isSemanticUsable ? "and semantic vector search" : "(keyword search active; semantic search disabled)"}. Always supply concise, targeted search terms for highest accuracy. You can optionally filter by version (e.g. "0.9.0", "0.8.0", "latest") or workspace project (e.g. "/", "assistant", "search").`,
        parameters: {
          type: "object",
          properties: {
            query: {
              type: "string",
              description: 'Targeted search query keywords or phrases (e.g. "installation", "quickstart", "api reference", "release notes", "configuration").'
            },
            version: {
              type: "string",
              description: 'Optional documentation version filter (e.g. "0.9.0", "0.8.0", "latest", "09", "08") to search exclusively in that version branch.'
            },
            project: {
              type: "string",
              description: 'Optional workspace project name or prefix filter (e.g. "/", "assistant", "search") to search within a specific project.'
            }
          },
          required: ["query"]
        },
        execute: async (rawArgs) => {
          const query = typeof rawArgs === "string" ? rawArgs : rawArgs?.query || rawArgs?.q || rawArgs?.search_query || rawArgs?.text || rawArgs?.input || "";
          const project = typeof rawArgs === "object" ? rawArgs?.project || rawArgs?.projectFilter : void 0;
          const version = typeof rawArgs === "object" ? rawArgs?.version || rawArgs?.versionFilter : void 0;
          return await this.searchAllWorkspaceIndexes(query, project, version);
        }
      });
      if (typeof document !== "undefined") {
        this.mount();
      }
    }
    renderSuggestionsHtml() {
      const cfg = window.__docmd_ai_config || {};
      const rawSuggestions = cfg.suggestions;
      const genericOptions = [
        { label: "How do I get started?", prompt: "How do I get started with this project?" },
        { label: "Key Features", prompt: "What are the main features documented here?" },
        { label: "Installation & Setup", prompt: "How do I install and set up this project?" },
        { label: "Configuration Options", prompt: "What configuration options are available?" },
        { label: "Quick Example", prompt: "Can you show me a quick usage example from the docs?" }
      ];
      let items = [];
      if (Array.isArray(rawSuggestions) && rawSuggestions.length > 0) {
        items = rawSuggestions.map((item) => {
          if (typeof item === "string") {
            return { label: item, prompt: item };
          }
          return {
            label: item.label || item.prompt || item.title || "Question",
            prompt: item.prompt || item.label || item.title || item
          };
        });
      } else {
        const shuffled = [...genericOptions].sort(() => Math.random() - 0.5);
        items = shuffled.slice(0, 2);
      }
      const buttons = items.map((item) => `<button class="docmd-ai-pill-btn" data-prompt="${this.escapeHtml(item.prompt)}">${this.escapeHtml(item.label)}</button>`).join("");
      return `<div class="docmd-ai-suggestions-row">${buttons}</div>`;
    }
    mount() {
      if (document.getElementById("docmd-ai-plugin-root")) return;
      const rawCfg = window.__docmd_ai_config || window.__DOCMD_AI_CONFIG__;
      if (!rawCfg || rawCfg.enabled === false || rawCfg.assistant === false || rawCfg.chat === false) return;
      const cfg = rawCfg;
      const i18n = window.__DOCMD_AI_I18N__ || {};
      const pos = cfg.position || "bottom-center";
      const placeholder = cfg.placeholder || i18n["ai.inputPlaceholder"] || "Ask AI Assistant...";
      const greeting = cfg.greeting || i18n["ai.greeting"] || "Hello! Ask me anything about this documentation.";
      const chatTitle = i18n["ai.chatTitle"] || "AI Assistant";
      const clearTitle = i18n["ai.clearChat"] || "Clear History";
      const closeTitle = i18n["ai.close"] || "Close AI Assistant";
      this.container = document.createElement("div");
      this.container.id = "docmd-ai-plugin-root";
      this.container.className = `pos-${pos}`;
      this.container.innerHTML = `
      <!-- Floating Bottom Prompt Bar -->
      <div class="docmd-ai-bar-wrap" id="docmd-ai-bar-wrap">
        <form class="docmd-ai-prompt-bar" id="docmd-ai-bar-form">
          <span class="docmd-ai-sparkle-icon">
            <svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2v4M12 18v4M4.93 4.93l2.83 2.83M16.24 16.24l2.83 2.83M2 12h4M18 12h4M4.93 19.07l2.83-2.83M16.24 7.76l2.83-2.83"/></svg>
          </span>
          <input type="text" class="docmd-ai-bar-input" id="docmd-ai-bar-input" placeholder="${placeholder}" autocomplete="off" />
          <span class="docmd-ai-shortcut-badge">\u2318I</span>
          <button type="submit" class="docmd-ai-submit-btn" title="Send (Enter)">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 10 4 15 9 20"/><path d="M20 4v7a4 4 0 0 1-4 4H4"/></svg>
          </button>
        </form>
      </div>

      <!-- Floating Sidebar Drawer -->
      <div class="docmd-ai-drawer" id="docmd-ai-drawer">
        <div class="docmd-ai-drawer-header">
          <div class="docmd-ai-header-left">
            <span class="docmd-ai-status-dot"></span>
            <span class="docmd-ai-title-text">${chatTitle}</span>
            <!--<span class="docmd-ai-badge-tag">docmd</span>-->
          </div>
          <div class="docmd-ai-header-right">
            <button class="docmd-ai-icon-action" id="docmd-ai-clear-btn" title="${clearTitle}">
              <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 6h18M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>
            </button>
            <button class="docmd-ai-icon-action" id="docmd-ai-close-btn" title="${closeTitle}">
              <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
            </button>
          </div>
        </div>

        <div class="docmd-ai-messages-list" id="docmd-ai-messages">
          <div class="docmd-ai-chat-bubble assistant">
            ${greeting}
            ${this.renderSuggestionsHtml()}
          </div>
        </div>

        <div class="docmd-ai-drawer-footer">
          <form class="docmd-ai-drawer-form" id="docmd-ai-drawer-form">
            <input type="text" class="docmd-ai-drawer-input" id="docmd-ai-drawer-input" placeholder="${placeholder}" autocomplete="off" />
            <button type="submit" class="docmd-ai-drawer-send-btn" title="Send (Enter)">
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 10 4 15 9 20"/><path d="M20 4v7a4 4 0 0 1-4 4H4"/></svg>
            </button>
          </form>
          <div class="docmd-ai-footer-branding">
            <a href="https://docmd.io/assistant/" target="_blank" rel="noopener" class="docmd-ai-footer-link">${i18n["ai.poweredBy"] || "Powered by docmd assistant"}</a>
          </div>
        </div>
      </div>
    `;
      document.body.appendChild(this.container);
      this.bindEvents();
    }
    bindEvents() {
      const barWrap = document.getElementById("docmd-ai-bar-wrap");
      const barForm = document.getElementById("docmd-ai-bar-form");
      const barInput = document.getElementById("docmd-ai-bar-input");
      const drawer = document.getElementById("docmd-ai-drawer");
      const closeBtn = document.getElementById("docmd-ai-close-btn");
      const clearBtn = document.getElementById("docmd-ai-clear-btn");
      const drawerForm = document.getElementById("docmd-ai-drawer-form");
      const drawerInput = document.getElementById("docmd-ai-drawer-input");
      barForm?.addEventListener("submit", (e) => {
        e.preventDefault();
        if (this.isPending) return;
        const query = barInput.value.trim();
        if (!query) return;
        barInput.value = "";
        this.openDrawer();
        this.submitQuery(query);
      });
      closeBtn?.addEventListener("click", () => this.closeDrawer());
      clearBtn?.addEventListener("click", () => this.clearChat());
      drawerForm?.addEventListener("submit", (e) => {
        e.preventDefault();
        if (this.isPending) return;
        const query = drawerInput.value.trim();
        if (!query) return;
        drawerInput.value = "";
        this.submitQuery(query);
      });
      document.addEventListener("keydown", (e) => {
        if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === "i") {
          e.preventDefault();
          if (this.isDrawerOpened) {
            this.closeDrawer();
          } else {
            this.openDrawer();
            drawerInput?.focus();
          }
        } else if (e.key === "Escape" && this.isDrawerOpened) {
          this.closeDrawer();
        }
      });
      const msgsContainer = document.getElementById("docmd-ai-messages");
      msgsContainer?.addEventListener("click", (e) => {
        const target = e.target;
        const copyBtn = target.closest(".docmd-ai-code-copy-btn");
        if (copyBtn) {
          const wrap = copyBtn.closest(".docmd-ai-code-wrap");
          const codeEl = wrap?.querySelector("code");
          if (codeEl) {
            const textToCopy = codeEl.textContent || "";
            const copySvg = `<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path></svg>`;
            const checkSvg = `<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"></polyline></svg>`;
            navigator.clipboard.writeText(textToCopy).then(() => {
              copyBtn.classList.add("copied");
              copyBtn.innerHTML = `${checkSvg}<span>Copied</span>`;
              setTimeout(() => {
                copyBtn.classList.remove("copied");
                copyBtn.innerHTML = `${copySvg}<span>Copy</span>`;
              }, 2e3);
            }).catch((err) => {
              console.error("[docmd-ai] Failed to copy code snippet:", err);
            });
          }
          return;
        }
        const msgCopyBtn = target.closest(".copy-msg-btn");
        if (msgCopyBtn) {
          const bubble = msgCopyBtn.closest(".docmd-ai-chat-bubble");
          const contentEl = bubble?.querySelector(".docmd-ai-content") || bubble;
          const textToCopy = contentEl ? (contentEl.textContent || "").trim() : "";
          if (textToCopy) {
            const checkSvg = `<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="20 6 9 17 4 12"></polyline></svg>`;
            const copySvg = `<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path></svg>`;
            navigator.clipboard.writeText(textToCopy).then(() => {
              msgCopyBtn.classList.add("copied");
              msgCopyBtn.innerHTML = `${checkSvg} <span>Copied</span>`;
              setTimeout(() => {
                msgCopyBtn.classList.remove("copied");
                msgCopyBtn.innerHTML = `${copySvg} <span>Copy</span>`;
              }, 2e3);
            }).catch((err) => {
              console.error("[docmd-ai] Failed to copy message:", err);
            });
          }
          return;
        }
        const retryBtn = target.closest(".retry-msg-btn");
        if (retryBtn) {
          if (this.isPending) return;
          const prompt = retryBtn.getAttribute("data-prompt");
          if (prompt) {
            this.submitQuery(prompt);
          }
          return;
        }
        const editPromptBtn = target.closest(".edit-prompt-btn");
        if (editPromptBtn) {
          const prompt = editPromptBtn.getAttribute("data-prompt");
          if (prompt && drawerInput) {
            drawerInput.value = prompt;
            drawerInput.focus();
          }
          return;
        }
        if (target && target.classList.contains("docmd-ai-pill-btn")) {
          if (this.isPending) return;
          const prompt = target.getAttribute("data-prompt");
          if (prompt) {
            this.openDrawer();
            this.submitQuery(prompt);
          }
        }
      });
    }
    setPendingState(pending) {
      this.isPending = pending;
      const barInput = document.getElementById("docmd-ai-bar-input");
      const barSubmitBtn = document.querySelector('#docmd-ai-bar-form button[type="submit"]');
      const drawerInput = document.getElementById("docmd-ai-drawer-input");
      const drawerSubmitBtn = document.querySelector('#docmd-ai-drawer-form button[type="submit"]');
      const pillBtns = document.querySelectorAll(".docmd-ai-pill-btn");
      if (barInput) barInput.disabled = pending;
      if (barSubmitBtn) barSubmitBtn.disabled = pending;
      if (drawerInput) drawerInput.disabled = pending;
      if (drawerSubmitBtn) drawerSubmitBtn.disabled = pending;
      pillBtns.forEach((btn) => {
        btn.disabled = pending;
        if (pending) btn.classList.add("disabled");
        else btn.classList.remove("disabled");
      });
      const barForm = document.getElementById("docmd-ai-bar-form");
      const drawerForm = document.getElementById("docmd-ai-drawer-form");
      if (pending) {
        barForm?.classList.add("pending");
        drawerForm?.classList.add("pending");
      } else {
        barForm?.classList.remove("pending");
        drawerForm?.classList.remove("pending");
      }
    }
    openDrawer() {
      this.isDrawerOpened = true;
      const barWrap = document.getElementById("docmd-ai-bar-wrap");
      const drawer = document.getElementById("docmd-ai-drawer");
      barWrap?.classList.add("hidden");
      drawer?.classList.add("open");
      if (typeof document !== "undefined" && document.body.classList.contains("tc-panel-open")) {
        document.body.classList.remove("tc-panel-open");
      }
    }
    closeDrawer() {
      this.isDrawerOpened = false;
      const barWrap = document.getElementById("docmd-ai-bar-wrap");
      const drawer = document.getElementById("docmd-ai-drawer");
      drawer?.classList.remove("open");
      barWrap?.classList.remove("hidden");
    }
    buildSystemPrompt() {
      const cfg = window.__docmd_ai_config || {};
      const currentUrl = typeof location !== "undefined" ? location.href : "";
      const siteTitle = cfg.siteTitle || (typeof document !== "undefined" ? document.title : "Documentation");
      const isWorkspace = !!(cfg.isWorkspace && Array.isArray(cfg.workspaceProjects) && cfg.workspaceProjects.length > 0);
      const getSiteBaseUrl = () => {
        const cfg2 = window.__docmd_ai_config || {};
        let base = cfg2.siteBase || "/";
        if (typeof location !== "undefined") {
          const isLocal = location.hostname === "localhost" || location.hostname === "127.0.0.1";
          if (isLocal && base !== "/" && !location.pathname.startsWith(base)) {
            base = "/";
          }
          return new URL(base.startsWith("/") ? base : "/" + base, location.origin).href;
        }
        return base;
      };
      const siteBaseUrl = getSiteBaseUrl();
      const versionsObj = cfg.versions || {};
      const hasVersions = !!(versionsObj && (Array.isArray(versionsObj.all) || versionsObj.current));
      const allVersions = Array.isArray(versionsObj.all) ? versionsObj.all : versionsObj.current ? [{ id: versionsObj.current, label: versionsObj.current, dir: `v${versionsObj.current}` }] : [];
      const defaultVer = allVersions.find((v) => v.id === versionsObj.current) || allVersions[0] || null;
      let activeVersion = defaultVer;
      if (typeof location !== "undefined" && allVersions.length > 0) {
        for (const v of allVersions) {
          const vDir = v.dir || v.id;
          if (location.pathname.includes(`/${vDir}/`)) {
            activeVersion = v;
            break;
          }
        }
      }
      const i18nObj = cfg.i18n || {};
      const hasLocales = !!(i18nObj && (Array.isArray(i18nObj.locales) || i18nObj.default));
      const defaultLocaleId = i18nObj.default || "en";
      const allLocales = Array.isArray(i18nObj.locales) ? i18nObj.locales : [{ id: defaultLocaleId, label: "Default" }];
      const defaultLocale = allLocales.find((l) => l.id === defaultLocaleId) || allLocales[0];
      let activeLocale = defaultLocale;
      if (typeof location !== "undefined" && allLocales.length > 0) {
        const pathParts = location.pathname.split("/");
        const foundLoc = allLocales.find((l) => pathParts.includes(l.id));
        if (foundLoc) activeLocale = foundLoc;
      }
      let currentProjectName = "Main Documentation";
      let currentProjectPrefix = "/";
      if (isWorkspace && Array.isArray(cfg.workspaceProjects)) {
        const matchProj = cfg.workspaceProjects.find((p) => {
          const pPrefix = (p.prefix || "/").replace(/^\/|\/$/g, "");
          return pPrefix && typeof location !== "undefined" && location.pathname.includes(`/${pPrefix}`);
        });
        if (matchProj) {
          currentProjectName = matchProj.name || matchProj.prefix;
          currentProjectPrefix = matchProj.prefix || "/";
        } else {
          const rootProj = cfg.workspaceProjects.find((p) => p.prefix === "/");
          if (rootProj) currentProjectName = rootProj.name || "Main Documentation";
        }
      }
      let workspaceContext = `
SITE & ENVIRONMENT CONTEXT:
- Site Title: "${siteTitle}"
- Site Base URL: ${siteBaseUrl}
- Current Page URL: ${currentUrl}
- Current Active Project: "${currentProjectName}" (Prefix: "${currentProjectPrefix}")`;
      if (hasVersions && defaultVer && activeVersion) {
        const versionsListStr = allVersions.map((v) => `${v.label}${v.id === defaultVer.id ? " (latest/default)" : ""}`).join(", ");
        workspaceContext += `
- Active Version: ${activeVersion.label} (Directory: "${activeVersion.dir || activeVersion.id}")
- Default / Latest Version: ${defaultVer.label}
- Available Versions: ${versionsListStr}`;
      }
      if (hasLocales && activeLocale) {
        const localesListStr = allLocales.map((l) => `${l.label} ("${l.id}")${l.id === defaultLocaleId ? " (default)" : ""}`).join(", ");
        workspaceContext += `
- Active Locale: ${activeLocale.label} ("${activeLocale.id}")
- Available Locales: ${localesListStr}`;
      }
      if (isWorkspace && Array.isArray(cfg.workspaceProjects)) {
        const projectsList = cfg.workspaceProjects.map((p, idx) => {
          const pName = p.name || p.prefix;
          const pPrefix = p.prefix || "/";
          const pAbsUrl = typeof location !== "undefined" ? new URL(pPrefix.replace(/^\//, ""), siteBaseUrl).href : pPrefix;
          const isCurrent = pName === currentProjectName ? " [CURRENT PAGE PROJECT]" : "";
          return `  ${idx + 1}. Project "${pName}" (Prefix: "${pPrefix}", URL: ${pAbsUrl})${isCurrent}`;
        }).join("\n");
        workspaceContext += `
- Multi-Project Workspace Setup: Active (${cfg.workspaceProjects.length} Projects)
- Available Workspace Projects:
${projectsList}`;
      }
      workspaceContext += `

CRITICAL SCOPE & NAVIGATION RULES:
1. SCOPE PRIORITIZATION: Prioritize answers using content from the Current Active Project ("${currentProjectName}")${hasVersions && activeVersion ? `, active version branch (${activeVersion.label})` : ""}${hasLocales && activeLocale ? `, and active language (${activeLocale.label})` : ""}.
2. STRICT ACTIVE / LATEST VERSION ONLY: ONLY cite, explain, recommend, and link to pages from the active version (${activeVersion?.label || defaultVer?.label}) or latest branch (${defaultVer?.label}). Never suggest, cite, or list deprecated historical versions unless the user explicitly asks for an older version.
3. AUTONOMOUS & PROACTIVE TOOL EXECUTION:
   - Always use your tools proactively. NEVER ask the user "Would you like me to search?" or "Should I check?". Directly invoke \`search_documentation\`, \`get_site_structure\`, or \`read_documentation_page\` to retrieve facts before answering.
   - For questions about releases, versions, updates, or changelogs, search with \`search_documentation\` to retrieve the relevant release notes.
4. ACCURATE HYPERLINKS: ALWAYS ground page hyperlinks strictly in real search results or valid project URLs (${siteBaseUrl}). Never invent or hallucinate invalid subpaths.
5. TOKEN EFFICIENCY & TARGETED RETRIEVAL:
   - Do not read entire documentation sets or fetch excessive pages unless necessary.
   - Use \`search_documentation\` first to identify the exact single page or section needed.
   - Only call \`read_documentation_page\` on that specific page when required to fetch precise code snippets or steps.
   - Keep answers clean, structured, and focused directly on what the user asked.
6. STRICT FACTUALITY & ARCHITECTURAL SYNTHESIS:
   - Ground all answers, configuration snippets, code examples, and commands strictly in verified facts retrieved from this documentation site.
   - NEVER guess or fabricate non-existent keys, options, or parameters.
   - When a user asks about a capability or concept that does not have a single dedicated configuration option (e.g. pinning an item, database authentication, etc.), explicitly explain that no dedicated setting exists for that exact query, explain the closest supported architectural mechanism or pattern (such as explicit navigation configuration via navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to the relevant documentation pages.
   - Never return an empty response or silence. Always provide an informative, grounded answer to the user.
7. VERSION FILTERING:
   - The \`search_documentation\` tool supports an optional \`version\` parameter. When the user asks about a specific version (e.g. v0.8.0), specify \`version\` to filter results strictly to that version branch.`;
      const defaultBasePrompt = `You are docmd assistant, the AI documentation guide for "${siteTitle}" \u2014 a professional, precise, and concise technical assistant.

CRITICAL CONSTRAINTS & BEHAVIORAL RULES:
1. IDENTITY: Your name is "docmd assistant". You are an expert AI documentation assistant dedicated to "${siteTitle}". If asked who you are, state that you are docmd assistant, serving the documentation for "${siteTitle}".
2. STRICT SCOPE & BOUNDARIES: Answer ONLY questions related to the software, tools, APIs, guides, and documentation provided on this site. Politely decline off-topic queries.
3. PROFESSIONAL & CONCISE: Provide direct, succinct, and professional answers. Do NOT use excessive emojis (keep emojis to a minimum or none). Avoid conversational fluff, boilerplate apologies, or asking for permission. Get straight to the answer.
4. STRICT FACTUALITY & ARCHITECTURAL SYNTHESIS:
   - Ground all answers, configuration snippets, code examples, and commands strictly in verified facts retrieved from this documentation site.
   - NEVER guess or fabricate non-existent keys, options, or parameters.
   - If the documentation does not have an explicit dedicated setting matching the exact query, clearly state this, synthesize the closest supported mechanism or architectural pattern (such as explicit navigation in navigation.json or docmd.config.json, frontmatter, or project settings), and guide the user to relevant pages.
   - Never return an empty response or silence. Always provide an informative, grounded answer.
5. TARGETED RETRIEVAL & MINIMAL TOKEN USAGE:
   - Only retrieve what is strictly necessary. Never attempt to read the entire documentation or fetch excessive pages.
   - Use \`search_documentation\` first with targeted keywords to locate the exact page.
   - Only invoke \`read_documentation_page\` when you need specific code blocks or configuration details from that single page.
6. TOOL SELECTION & EXECUTION:
   - Use \`get_site_structure\` whenever you need structural inspection of available documentation versions, supported locales, or navigation trees.
   - Use \`search_documentation\` to search documentation content for specific technical terms, API parameters, error messages, or release notes. Keyword search is always active; pass clean, focused search terms for highest accuracy.
   - Use \`read_documentation_page\` when you need full section context or deep code examples.
7. CLEAN WRITING & LIST FORMATTING:
   - Write cleanly and directly without artificial gaps, repeated quotes, or messy text breaks.
   - For lists, use standard numbered lists (1., 2., 3.) or bullet points (-). Do not leave blank lines between list items unless separating distinct multi-paragraph steps.
8. HYPERLINKS & CITATIONS: Always include clickable Markdown hyperlinks \`[Page Title](path)\` in your response for referenced pages.`;
      const basePrompt = cfg.systemPrompt || defaultBasePrompt;
      return `${basePrompt}

${workspaceContext}`;
    }
    getSiteStructure() {
      const cfg = window.__docmd_ai_config || {};
      return {
        siteTitle: cfg.siteTitle || "Documentation",
        siteBaseUrl: cfg.siteUrl || cfg.siteBase || "/",
        currentUrl: typeof location !== "undefined" ? location.href : "",
        versions: cfg.versions || null,
        locales: cfg.i18n || null,
        searchCapabilities: cfg.searchCapabilities || { keyword: true, semantic: false },
        isWorkspace: !!cfg.isWorkspace,
        workspaceProjects: cfg.workspaceProjects || [],
        navigation: cfg.navigation || []
      };
    }
    async searchAllWorkspaceIndexes(rawQuery, projectFilter, versionFilter) {
      const hits = [];
      const query = typeof rawQuery === "string" ? rawQuery : rawQuery?.query || rawQuery?.q || rawQuery?.search_query || rawQuery?.text || rawQuery?.input || "";
      const cleanQuery = (query || "").trim();
      if (!cleanQuery) return [];
      const cleanQueryLower = cleanQuery.toLowerCase();
      const cfg = window.__docmd_ai_config || {};
      const getSiteBaseUrl = () => {
        const cfg2 = window.__docmd_ai_config || {};
        let base = cfg2.siteBase || "/";
        if (typeof location !== "undefined") {
          const isLocal = location.hostname === "localhost" || location.hostname === "127.0.0.1";
          if (isLocal && base !== "/" && !location.pathname.startsWith(base)) {
            base = "/";
          }
          return new URL(base.startsWith("/") ? base : "/" + base, location.origin).href;
        }
        return base;
      };
      const siteBaseUrl = getSiteBaseUrl();
      const versionsObj = cfg.versions || {};
      const allVerList = Array.isArray(versionsObj.all) ? versionsObj.all : [];
      const currentVerId = String(versionsObj.current || "");
      const currentVerDir = versionsObj.current ? allVerList.find((v) => v.id === versionsObj.current)?.dir || `v${versionsObj.current}` : "";
      let explicitVersionId = null;
      let explicitVersionDir = null;
      let explicitVersionLabel = null;
      if (versionFilter) {
        const vClean = String(versionFilter).toLowerCase().trim().replace(/^v/, "");
        if (vClean === "latest" || vClean === "current" || vClean === currentVerId.replace(/^v/, "")) {
          explicitVersionId = currentVerId;
          explicitVersionDir = currentVerDir;
        } else {
          const found = allVerList.find((v) => {
            const vid = String(v.id || "").toLowerCase().replace(/^v/, "");
            const vdir = String(v.dir || "").toLowerCase().replace(/^v/, "");
            const vlbl = String(v.label || "").toLowerCase().replace(/^v/, "");
            return vid === vClean || vdir === vClean || vlbl === vClean || vlbl.startsWith(vClean);
          });
          if (found) {
            explicitVersionId = String(found.id);
            explicitVersionDir = String(found.dir || `v${found.id}`);
            explicitVersionLabel = String(found.label || found.id);
          } else {
            explicitVersionId = vClean;
          }
        }
      }
      const olderVerTokens = [];
      for (const v of allVerList) {
        if (String(v.id) !== currentVerId) {
          if (v.id) olderVerTokens.push(String(v.id).toLowerCase());
          if (v.dir) olderVerTokens.push(String(v.dir).toLowerCase());
          if (v.label) {
            olderVerTokens.push(String(v.label).toLowerCase());
            olderVerTokens.push(String(v.label).replace(/^v/i, "").toLowerCase());
          }
        }
      }
      const isExplicitOlderVerRequest = explicitVersionId ? explicitVersionId !== currentVerId : olderVerTokens.some((tok) => cleanQueryLower.includes(tok));
      const i18nObj = cfg.i18n || {};
      const allLocales = Array.isArray(i18nObj.locales) ? i18nObj.locales : [];
      let activeLocaleId = i18nObj.default || "en";
      if (typeof location !== "undefined") {
        const pathParts = location.pathname.split("/");
        const foundLoc = allLocales.find((l) => pathParts.includes(l.id));
        if (foundLoc) activeLocaleId = foundLoc.id;
      }
      const nonActiveLocaleIds = allLocales.filter((l) => l.id !== activeLocaleId).map((l) => l.id.toLowerCase());
      const isExplicitLocaleRequest = nonActiveLocaleIds.some((locId) => cleanQueryLower.includes(locId));
      const isPathExcluded = (rawId, itemVersion) => {
        const norm = String(rawId || "").replace(/^\//, "").toLowerCase();
        const itemVerLower = String(itemVersion || "").toLowerCase().replace(/^v/, "");
        if (explicitVersionId) {
          if (explicitVersionId === currentVerId) {
            for (const tok of olderVerTokens) {
              if (norm === tok || norm.startsWith(`${tok}/`) || norm.includes(`/${tok}/`)) return true;
            }
          } else {
            const tokens = [explicitVersionId.toLowerCase(), `v${explicitVersionId.toLowerCase()}`];
            if (explicitVersionDir) tokens.push(explicitVersionDir.toLowerCase());
            if (explicitVersionLabel) tokens.push(explicitVersionLabel.toLowerCase().replace(/^v/, ""));
            const matchesExplicit = tokens.some((tok) => norm === tok || norm.startsWith(`${tok}/`) || norm.includes(`/${tok}/`) || itemVerLower === tok);
            if (!matchesExplicit) return true;
          }
        } else if (!isExplicitOlderVerRequest) {
          for (const tok of olderVerTokens) {
            if (norm === tok || norm.startsWith(`${tok}/`) || norm.includes(`/${tok}/`)) {
              return true;
            }
          }
        }
        if (!isExplicitLocaleRequest) {
          for (const loc of nonActiveLocaleIds) {
            if (norm === loc || norm.startsWith(`${loc}/`) || norm.includes(`/${loc}/`)) {
              return true;
            }
          }
        }
        return false;
      };
      const STOP_WORDS = /* @__PURE__ */ new Set([
        "a",
        "an",
        "and",
        "are",
        "as",
        "at",
        "be",
        "by",
        "for",
        "from",
        "has",
        "he",
        "in",
        "is",
        "it",
        "its",
        "of",
        "on",
        "that",
        "the",
        "to",
        "was",
        "were",
        "will",
        "with",
        "what",
        "why",
        "how",
        "can",
        "you",
        "me",
        "i",
        "do",
        "does",
        "did",
        "than"
      ]);
      const rawTokens = cleanQueryLower.replace(/[\-_.]/g, " ").split(/\s+/).filter((t) => t.length > 0);
      const queryTokens = rawTokens.filter((t) => !STOP_WORDS.has(t));
      const versionMatches = cleanQuery.match(/\d+[\.\-_]\d+([\.\-_]\d+)?/g);
      try {
        if (window.docmdSearch && typeof window.docmdSearch.search === "function") {
          const searchOpts = {};
          if (explicitVersionId) {
            searchOpts.version = explicitVersionId;
          }
          let localHits = await window.docmdSearch.search(query, searchOpts);
          const isComparisonQuery = /\b(better|advantages?|versus|vs|compare|comparison)\b/i.test(cleanQueryLower);
          if (isComparisonQuery && !cleanQueryLower.includes("comparison")) {
            try {
              const compHits = await window.docmdSearch.search("comparison", searchOpts);
              if (Array.isArray(compHits)) {
                if (!Array.isArray(localHits)) localHits = [];
                for (const ch of compHits) {
                  if (!localHits.some((lh) => lh.id === ch.id)) {
                    localHits.unshift({ ...ch, score: 999 });
                  }
                }
              }
            } catch {
            }
          }
          if (Array.isArray(localHits)) {
            const pageMap = /* @__PURE__ */ new Map();
            const filtered = localHits.filter((item) => !isPathExcluded(item.id || item.url || "", item.version));
            for (const item of filtered) {
              const rawId = String(item.id || item.url || "");
              const cleanId = rawId.startsWith("/") ? rawId.slice(1) : rawId;
              const baseId = cleanId.split("#")[0];
              const isHeading = cleanId.includes("#");
              const titleLower = String(item.title || cleanId).toLowerCase();
              const textLower = String(item.text || item.snippet || "").toLowerCase();
              const idLower = cleanId.toLowerCase();
              let score = typeof item.score === "number" ? item.score : 1;
              for (const tok of queryTokens.length > 0 ? queryTokens : rawTokens) {
                if (titleLower.includes(tok)) score += 25;
                if (idLower.includes(tok)) score += 15;
                if (textLower.includes(tok)) score += 2;
              }
              if (versionMatches) {
                for (const vm of versionMatches) {
                  const normV = vm.replace(/[\-_]/g, ".");
                  const dashV = vm.replace(/[\.]/g, "-");
                  if (idLower.includes(`/${dashV}/`) || idLower.endsWith(`/${dashV}`) || idLower.includes(`-${dashV}-`)) {
                    score += 600;
                  } else if (titleLower.includes(`v${normV}`) || titleLower.includes(`v${dashV}`) || titleLower.includes(` ${normV} `) || titleLower.includes(` ${normV}-`)) {
                    score += 500;
                  } else if (titleLower.includes(normV) || idLower.includes(dashV)) {
                    score += 150;
                  }
                }
              }
              if (isHeading) score -= 10;
              const fullUrl = cleanId.startsWith("http") ? cleanId : new URL(cleanId, siteBaseUrl).href;
              const existing = pageMap.get(baseId);
              if (!existing || existing.score < score) {
                pageMap.set(baseId, { item, score, cleanId, fullUrl });
              }
            }
            const rankedEntries = Array.from(pageMap.values()).sort((a, b) => b.score - a.score);
            for (const entry of rankedEntries) {
              const { item, cleanId, fullUrl } = entry;
              if (!hits.some((existing) => existing.url === fullUrl)) {
                hits.push({
                  project: "Current Project",
                  title: item.title || cleanId,
                  url: fullUrl,
                  snippet: item.snippet || item.text || "",
                  searchType: "keyword"
                });
              }
              if (hits.length >= 6) break;
            }
          }
        }
      } catch {
      }
      if (cfg.isWorkspace && Array.isArray(cfg.workspaceProjects)) {
        for (const p of cfg.workspaceProjects) {
          if (projectFilter && p.prefix !== projectFilter && p.name !== projectFilter) continue;
          try {
            const pPrefix = p.prefix || "/";
            const pBaseUrl = new URL(pPrefix.replace(/^\//, ""), siteBaseUrl).href.replace(/\/?$/, "/");
            const searchIndexPath = `${pBaseUrl}_docmd-search/search-index.json`;
            const res = await fetch(searchIndexPath);
            if (res.ok) {
              const indexData = await res.json();
              const docs = indexData.storedFields ? Object.values(indexData.storedFields) : Array.isArray(indexData) ? indexData : [];
              const filteredDocs = docs.filter((doc) => {
                const rawId = String(doc.id || doc.url || "");
                return !isPathExcluded(rawId, doc.version);
              });
              const pageMap = /* @__PURE__ */ new Map();
              for (const doc of filteredDocs) {
                const titleStr = String(doc.title || doc.id || "").toLowerCase();
                const textStr = String(doc.text || "").toLowerCase();
                const rawId = String(doc.id || "");
                const cleanId = rawId.startsWith("/") ? rawId.slice(1) : rawId;
                const baseId = cleanId.split("#")[0];
                const isHeading = cleanId.includes("#");
                let score = 0;
                for (const term of queryTokens.length > 0 ? queryTokens : rawTokens) {
                  if (titleStr.includes(term)) score += 25;
                  if (rawId.toLowerCase().includes(term)) score += 15;
                  if (textStr.includes(term)) score += 2;
                }
                if (versionMatches) {
                  for (const vm of versionMatches) {
                    const normV = vm.replace(/[\-_]/g, ".");
                    const dashV = vm.replace(/[\.]/g, "-");
                    if (rawId.toLowerCase().includes(`/${dashV}/`) || rawId.toLowerCase().endsWith(`/${dashV}`)) {
                      score += 600;
                    } else if (titleStr.includes(`v${normV}`) || titleStr.includes(`v${dashV}`)) {
                      score += 500;
                    } else if (titleStr.includes(normV) || rawId.toLowerCase().includes(dashV)) {
                      score += 150;
                    }
                  }
                }
                if (currentVerDir && rawId.includes(`/${currentVerDir}/`)) score += 10;
                if (activeLocaleId && rawId.includes(`/${activeLocaleId}/`)) score += 5;
                if (isHeading) score -= 10;
                const fullUrl = rawId.startsWith("http") ? rawId : new URL(cleanId, pBaseUrl).href;
                const existing = pageMap.get(baseId);
                if (!existing || existing.score < score) {
                  pageMap.set(baseId, { doc, score, fullUrl });
                }
              }
              const scored = Array.from(pageMap.values()).filter((h) => h.score > 0).sort((a, b) => b.score - a.score);
              for (const hit of scored.slice(0, 3)) {
                const { doc, fullUrl } = hit;
                const rawId = doc.id || "";
                if (!hits.some((existing) => existing.url === fullUrl)) {
                  hits.push({
                    project: p.name || p.prefix,
                    title: doc.title || rawId,
                    url: fullUrl,
                    snippet: (doc.text || "").slice(0, 150) + "...",
                    searchType: "keyword"
                  });
                }
              }
            }
          } catch {
          }
        }
      }
      return hits;
    }
    async fetchLocalSearchContext(query) {
      const trimmed = query.trim().toLowerCase();
      const isGreetingOrCasual = /^(hi|hello|hey|howdy|greetings|good\s+(morning|afternoon|evening|day)|who\s+are\s+you|what\s+can\s+you\s+do|help|thanks|thank\s+you|bye|goodbye)[!?. ]*$/i.test(trimmed) || trimmed.length <= 2;
      if (isGreetingOrCasual) {
        return "";
      }
      try {
        const hits = await this.searchAllWorkspaceIndexes(query);
        if (Array.isArray(hits) && hits.length > 0) {
          const formattedHits = hits.slice(0, 5).map((hit) => {
            return `- [${hit.project}] ${hit.title} (${hit.searchType}): ${hit.url}
  Snippet: ${hit.snippet}`;
          }).join("\n");
          return `

[Documentation Search Context (Multi-Project Workspace)]:
${formattedHits}`;
        }
      } catch {
      }
      return "";
    }
    renderUnconfiguredNotice(data) {
      const title = data?.title || "Connect Your AI Assistant";
      const message = data?.message || "Add your free AI relay or BYOK API key on docmd Cloud to enable the assistant for your visitors.";
      const configUrl = data?.configUrl || "https://cloud.docmd.io";
      const features = Array.isArray(data?.features) ? data.features : [
        "**Free AI relay** \u2014 bring your own API key for OpenAI, Anthropic, Gemini, DeepSeek, or Ollama.",
        "**Query analytics** \u2014 see what your visitors are asking in real time.",
        "**Setup takes under a minute** \u2014 just add your `projectId` to `docmd.config.json`."
      ];
      const featureItemsHtml = features.map((f) => `<li>${this.formatMarkdown(f)}</li>`).join("");
      const msgs = document.getElementById("docmd-ai-messages");
      const div = document.createElement("div");
      div.className = "docmd-ai-chat-bubble assistant";
      div.innerHTML = `
      <div class="docmd-ai-unconfigured-card">
        <div class="docmd-ai-unconfigured-title">
          <svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2v4M12 18v4M4.93 4.93l2.83 2.83M16.24 16.24l2.83 2.83M2 12h4M18 12h4M4.93 19.07l2.83-2.83M16.24 7.76l2.83-2.83"></path></svg>
          ${this.escapeHtml(title)}
        </div>
        <p>${this.escapeHtml(message)}</p>
        <ul class="docmd-ai-unconfigured-list">
          ${featureItemsHtml}
        </ul>
        <a href="${this.escapeHtml(configUrl)}" target="_blank" rel="noopener" class="docmd-ai-unconfigured-btn">
          Connect on docmd Cloud \u2192
        </a>
      </div>
    `;
      if (msgs) {
        msgs.appendChild(div);
        msgs.scrollTop = msgs.scrollHeight;
      }
    }
    getStatusSvgIcon(iconName) {
      switch (iconName) {
        case "search":
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>`;
        case "folder-tree":
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"></path><line x1="12" y1="11" x2="12" y2="17"></line><line x1="9" y1="14" x2="15" y2="14"></line></svg>`;
        case "book-open":
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 3h6a4 4 0 0 1 4 4v14a3 3 0 0 0-3-3H2z"></path><path d="M22 3h-6a4 4 0 0 0-4 4v14a3 3 0 0 1 3-3h7z"></path></svg>`;
        case "navigation":
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="3 11 22 2 13 21 11 13 3 11"></polygon></svg>`;
        case "copy":
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path></svg>`;
        case "cog":
          return `<svg class="docmd-ai-spin" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>`;
        case "brain":
        case "sparkles":
        default:
          return `<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2v4M12 18v4M4.93 4.93l2.83 2.83M16.24 16.24l2.83 2.83M2 12h4M18 12h4M4.93 19.07l2.83-2.83M16.24 7.76l2.83-2.83"></path></svg>`;
      }
    }
    async submitQuery(text) {
      if (this.isPending) return;
      this.setPendingState(true);
      this.appendMsg("user", text, true);
      const msgs = document.getElementById("docmd-ai-messages");
      const bubble = document.createElement("div");
      bubble.className = "docmd-ai-chat-bubble assistant";
      const statusWrap = document.createElement("div");
      statusWrap.className = "docmd-ai-status-badge";
      statusWrap.innerHTML = `${this.getStatusSvgIcon("brain")} <span>Thinking...</span>`;
      bubble.appendChild(statusWrap);
      const contentDiv = document.createElement("div");
      contentDiv.className = "docmd-ai-content";
      bubble.appendChild(contentDiv);
      if (msgs) {
        msgs.appendChild(bubble);
        msgs.scrollTop = msgs.scrollHeight;
      }
      let accumulatedText = "";
      try {
        const docContext = await this.fetchLocalSearchContext(text);
        const queryWithContext = docContext ? `${text}${docContext}` : text;
        let res;
        if (typeof this.engine.sendMessageStream === "function") {
          res = await this.engine.sendMessageStream(queryWithContext, {
            onStatus: (status) => {
              if (statusWrap && status) {
                const statusObj = typeof status === "string" ? { text: status } : { ...status };
                let statusText = statusObj.text || "Thinking...";
                let statusIcon = statusObj.icon;
                if (/^Running\s+([a-z0-9_]+)\.\.\./i.test(statusText)) {
                  const tool = statusText.match(/^Running\s+([a-z0-9_]+)\.\.\./i)?.[1] || "";
                  if (tool === "read_documentation_page") {
                    statusText = "Reading documentation...";
                    statusIcon = statusIcon || "book-open";
                  } else if (tool === "search_documentation") {
                    statusText = "Searching documentation...";
                    statusIcon = statusIcon || "search";
                  } else if (tool === "navigate_to_page") {
                    statusText = "Navigating to page...";
                    statusIcon = statusIcon || "navigation";
                  } else if (tool === "get_site_structure") {
                    statusText = "Inspecting site navigation & structure...";
                    statusIcon = statusIcon || "folder-tree";
                  } else if (tool === "copy_code_snippet") {
                    statusText = "Copying code snippet...";
                    statusIcon = statusIcon || "copy";
                  } else {
                    const readable = tool.replace(/^([a-z])/, (m) => m.toUpperCase()).replace(/_/g, " ");
                    statusText = `${readable}...`;
                  }
                } else if (/^[a-z]+(_[a-z0-9]+)+$/.test(statusText)) {
                  if (statusText === "read_documentation_page") {
                    statusText = "Reading documentation...";
                    statusIcon = statusIcon || "book-open";
                  } else if (statusText === "search_documentation") {
                    statusText = "Searching documentation...";
                    statusIcon = statusIcon || "search";
                  } else if (statusText === "navigate_to_page") {
                    statusText = "Navigating to page...";
                    statusIcon = statusIcon || "navigation";
                  } else if (statusText === "get_site_structure") {
                    statusText = "Inspecting site navigation & structure...";
                    statusIcon = statusIcon || "folder-tree";
                  } else if (statusText === "copy_code_snippet") {
                    statusText = "Copying code snippet...";
                    statusIcon = statusIcon || "copy";
                  } else {
                    const readable = statusText.replace(/^([a-z])/, (m) => m.toUpperCase()).replace(/_/g, " ");
                    statusText = `${readable}...`;
                  }
                }
                statusWrap.style.display = "inline-flex";
                statusWrap.innerHTML = `${this.getStatusSvgIcon(statusIcon)} <span>${this.escapeHtml(statusText)}</span>`;
                if (msgs) msgs.scrollTop = msgs.scrollHeight;
              }
            },
            onChunk: (chunk, meta) => {
              if (chunk) {
                if (meta?.replace) {
                  accumulatedText = chunk;
                } else if (!accumulatedText) {
                  accumulatedText = chunk;
                } else if (chunk.startsWith(accumulatedText)) {
                  accumulatedText = chunk;
                } else {
                  accumulatedText += chunk;
                }
              }
              if (statusWrap) {
                statusWrap.style.display = "none";
              }
              contentDiv.innerHTML = this.formatMarkdown(accumulatedText);
              if (msgs) msgs.scrollTop = msgs.scrollHeight;
            }
          });
        } else {
          res = await this.engine.sendMessage(queryWithContext);
        }
        if (res && res.unconfigured) {
          bubble.remove();
          this.renderUnconfiguredNotice(res.unconfiguredData || res);
          return;
        }
        if (statusWrap) {
          statusWrap.style.display = "none";
        }
        const fallbackMsg = text ? `No documentation page in the retrieved results explicitly covers a dedicated setting for "${text}". You can explore the site navigation or search for related topics.` : "No response generated.";
        const finalMsg = res?.message || accumulatedText || fallbackMsg;
        contentDiv.innerHTML = this.formatMarkdown(finalMsg);
        const cfg = window.__docmd_ai_config || {};
        if (cfg.messageActions) {
          const actionsDiv = document.createElement("div");
          actionsDiv.className = "docmd-ai-bubble-actions";
          actionsDiv.innerHTML = `
          <button class="docmd-ai-msg-action-btn copy-msg-btn" title="Copy response">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path></svg>
            <span>Copy</span>
          </button>
          <button class="docmd-ai-msg-action-btn retry-msg-btn" title="Retry prompt" data-prompt="${this.escapeHtml(text)}">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="1 4 1 10 7 10"></polyline><path d="M3.51 15a9 9 0 1 0 2.13-9.36L1 10"></path></svg>
            <span>Retry</span>
          </button>
        `;
          bubble.appendChild(actionsDiv);
        }
        if (msgs) msgs.scrollTop = msgs.scrollHeight;
      } catch (err) {
        const errMsg = err?.message || String(err || "");
        const isAuthOrConfigError = errMsg.includes("Domain Not Authorized") || errMsg.includes("Origin is not authorized") || errMsg.includes("403") || errMsg.includes("401") || err?.unconfigured;
        if (isAuthOrConfigError) {
          bubble.remove();
          this.renderUnconfiguredNotice({
            title: "Domain Not Authorized",
            message: errMsg || "Origin is not authorized for the selected docmd Cloud project.",
            features: [
              "**Free AI relay** \u2014 bring your own API key for OpenAI, Anthropic, Gemini, DeepSeek, or Ollama.",
              "**Query analytics** \u2014 see what your visitors are asking in real time.",
              "**Setup takes under a minute** \u2014 just add your `projectId` to `docmd.config.json`."
            ]
          });
        } else {
          if (statusWrap) statusWrap.style.display = "none";
          contentDiv.innerHTML = `<span style="color: var(--ai-text-muted);">Sorry, I encountered an issue processing your request: ${this.escapeHtml(errMsg)}</span>`;
        }
      } finally {
        this.setPendingState(false);
      }
    }
    clearChat() {
      this.engine.clearHistory();
      const msgs = document.getElementById("docmd-ai-messages");
      if (msgs) {
        msgs.innerHTML = `
        <div class="docmd-ai-chat-bubble assistant">
          Conversation cleared. How else can I help you?
          ${this.renderSuggestionsHtml()}
        </div>
      `;
      }
    }
    appendMsg(sender, text, save = true) {
      const msgs = document.getElementById("docmd-ai-messages");
      const div = document.createElement("div");
      div.className = `docmd-ai-chat-bubble ${sender}`;
      if (sender === "user") {
        const cfg = window.__docmd_ai_config || {};
        const editBtnHtml = cfg.messageActions ? `<button class="edit-prompt-btn" title="Edit and re-run prompt" data-prompt="${this.escapeHtml(text)}">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 20h9"/><path d="M16.5 3.5a2.121 2.121 0 0 1 3 3L7 19l-4 1 1-4L16.5 3.5z"/></svg>
          </button>` : "";
        div.innerHTML = `<span class="docmd-ai-user-text">${this.escapeHtml(text)}</span>${editBtnHtml}`;
      } else {
        div.innerHTML = this.formatMarkdown(text);
      }
      if (msgs) {
        msgs.appendChild(div);
        msgs.scrollTop = msgs.scrollHeight;
      }
      return div;
    }
    escapeHtml(raw) {
      return (raw || "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    }
    formatMarkdown(raw) {
      if (!raw) return "";
      let cleaned = raw.replace(/<(?:[a-zA-Z0-9_\-]+:)?(think|thought|reasoning|reflection|plan)\b[^>]*>[\s\S]*?<\/(?:[a-zA-Z0-9_\-]+:)?\1>/gi, "").replace(/```(?:thought|thinking|reasoning|reflection)\s*\n[\s\S]*?```/gi, "").replace(/\[(?:thought|thinking|reasoning):\s*[\s\S]*?\]/gi, "").replace(/\]<\][a-zA-Z0-9_\-]+\[>\[[\s\S]*?(?:<\/(?:request|tool_call|action)>|\]<\][a-zA-Z0-9_\-]+\[>\[|$)/gi, "").replace(/\]<\][a-zA-Z0-9_\-]+\[>\[/gi, "").replace(/<\/?(?:[a-zA-Z0-9_\-]+:)?(?:think|thought|reasoning|reflection|plan)\b[^>]*>/gi, "").replace(/<(?:[a-zA-Z0-9_\-]+:)?(tool_call|function_call|tool|action|request)\b[^>]*>[\s\S]*?<\/(?:[a-zA-Z0-9_\-]+:)?\1>/gi, "").replace(/```(?:tool_call|function_call|tool|action|json:tool)\s*\n[\s\S]*?```/gi, "").replace(/\{\s*"(?:name|tool|action|function)"\s*:\s*"[^"]+"\s*,\s*"(?:parameters|arguments|args|input)"\s*:\s*\{[\s\S]*?\}\s*\}/g, "").replace(/<\/?(?:[a-zA-Z0-9_\-]+:)?(?:tool_call|function_call|tool|action|request)\b[^>]*>/gi, "").trim();
      if (!cleaned) cleaned = raw;
      const cfg = window.__docmd_ai_config || {};
      const getSiteBaseUrl = () => {
        const cfg2 = window.__docmd_ai_config || {};
        let base = cfg2.siteBase || "/";
        if (typeof location !== "undefined") {
          const isLocal2 = location.hostname === "localhost" || location.hostname === "127.0.0.1";
          if (isLocal2 && base !== "/" && !location.pathname.startsWith(base)) {
            base = "/";
          }
          return new URL(base.startsWith("/") ? base : "/" + base, location.origin).href;
        }
        return base;
      };
      const siteBaseUrl = getSiteBaseUrl();
      const resolveCanonicalUrl = (href) => {
        if (!href) return "#";
        let targetUrl = href.trim();
        const subpathBase2 = (cfg.siteBase || "").replace(/^\/|\/$/g, "");
        const isLocal2 = typeof location !== "undefined" && (location.hostname === "localhost" || location.hostname === "127.0.0.1");
        if (isLocal2 && subpathBase2 && !location.pathname.startsWith("/" + subpathBase2)) {
          targetUrl = targetUrl.replace(new RegExp(`/${subpathBase2}(/|$)`, "g"), "/");
        }
        if (/^(?:https?:|mailto:|tel:)/i.test(targetUrl)) return targetUrl;
        if (targetUrl.startsWith("#")) return `${siteBaseUrl}${targetUrl}`;
        try {
          const cleanHref = targetUrl.startsWith("/") ? targetUrl.slice(1) : targetUrl;
          return new URL(cleanHref, siteBaseUrl).href;
        } catch {
          return targetUrl;
        }
      };
      const codeBlocks = [];
      const copySvg = `<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path></svg>`;
      const renderCodeBlock = (lang, code) => {
        const cleanLang = (lang || "").trim().toLowerCase();
        const langSpan = cleanLang ? `<span class="docmd-ai-code-lang">${cleanLang}</span>` : `<span class="docmd-ai-code-lang">code</span>`;
        const copyBtn = `<button class="docmd-ai-code-copy-btn" type="button" title="Copy code" aria-label="Copy code">${copySvg}<span>Copy</span></button>`;
        const headerStr = `<div class="docmd-ai-code-header">${langSpan}${copyBtn}</div>`;
        const escapedCode = code.trim().replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
        return `<div class="docmd-ai-code-wrap">${headerStr}<pre><code>${escapedCode}</code></pre></div>`;
      };
      cleaned = cleaned.replace(/(?:^|\n)([ \t]*)(`{4,}|~{4,})([ \t]*\S*.*)\r?\n([\s\S]*?)\r?\n\1\2[ \t]*(?=\r?\n|$)/g, (_match, _indent, _fence, lang, code) => {
        const placeholder = `__CODE_BLOCK_${codeBlocks.length}__`;
        codeBlocks.push(renderCodeBlock(lang.trim(), code));
        return `
${placeholder}
`;
      });
      cleaned = cleaned.replace(/(?:^|\n)([ \t]*)(`{3}|~{3})([ \t]*\S*.*)\r?\n([\s\S]*?)\r?\n\1\2[ \t]*(?=\r?\n|$)/g, (_match, _indent, _fence, lang, code) => {
        const placeholder = `__CODE_BLOCK_${codeBlocks.length}__`;
        codeBlocks.push(renderCodeBlock(lang.trim(), code));
        return `
${placeholder}
`;
      });
      cleaned = cleaned.replace(/(`{3,})(\w+)?[ \t]*\r?\n([\s\S]*?)\1/g, (_match, _fence, lang, code) => {
        const placeholder = `__CODE_BLOCK_${codeBlocks.length}__`;
        codeBlocks.push(renderCodeBlock(lang || "", code));
        return placeholder;
      });
      cleaned = cleaned.replace(/(`{3,})(\w+)([ \t]+[^\n][\s\S]*?)\1/g, (_match, _fence, lang, code) => {
        const placeholder = `__CODE_BLOCK_${codeBlocks.length}__`;
        codeBlocks.push(renderCodeBlock(lang || "", code));
        return placeholder;
      });
      let text = this.escapeHtml(cleaned);
      text = text.replace(/^#### (.*$)/gim, "<h5>$1</h5>");
      text = text.replace(/^### (.*$)/gim, "<h4>$1</h4>");
      text = text.replace(/^## (.*$)/gim, "<h3>$1</h3>");
      text = text.replace(/^# (.*$)/gim, "<h3>$1</h3>");
      const parseListItems = (rawListText, isOrdered) => {
        const lines = rawListText.split(/\r?\n/);
        const items = [];
        let currentItemText = "";
        const itemRegex = isOrdered ? /^\s*(\d+)[\.\)]\s+(.*)$/ : /^\s*[-*+]\s+(.*)$/;
        for (const line of lines) {
          const mainMatch = line.match(itemRegex);
          if (mainMatch) {
            if (currentItemText) {
              items.push(`<li>${currentItemText.trim()}</li>`);
            }
            currentItemText = isOrdered ? mainMatch[2] : mainMatch[1];
          } else if (line.trim().length === 0) {
            continue;
          } else if (currentItemText) {
            const subBullet = line.match(/^\s+[-*+]\s+(.*)$/);
            if (subBullet) {
              currentItemText += `<br/><span class="docmd-ai-sub-item">\u2022 ${subBullet[1].trim()}</span>`;
            } else {
              currentItemText += " " + line.trim();
            }
          }
        }
        if (currentItemText) {
          items.push(`<li>${currentItemText.trim()}</li>`);
        }
        if (items.length === 0) return rawListText;
        const tag = isOrdered ? "ol" : "ul";
        return `

<${tag}>${items.join("")}</${tag}>

`;
      };
      const orderedListRegex = /(?:^[ \t]*\d+[\.\)][ \t]+.*(?:\r?\n|$))(?:[ \t]*(?:\r?\n)|[ \t]+\S.*(?:\r?\n|$)|^[ \t]*\d+[\.\)][ \t]+.*(?:\r?\n|$))*/gm;
      text = text.replace(orderedListRegex, (match) => parseListItems(match, true));
      const unorderedListRegex = /(?:^[ \t]*[-*+][ \t]+.*(?:\r?\n|$))(?:[ \t]*(?:\r?\n)|[ \t]+\S.*(?:\r?\n|$)|^[ \t]*[-*+][ \t]+.*(?:\r?\n|$))*/gm;
      text = text.replace(unorderedListRegex, (match) => parseListItems(match, false));
      text = text.replace(/(?:^\s*&gt;\s+.*(?:\r?\n|$))+/gm, (match) => {
        const content = match.replace(/^\s*&gt;\s+/gm, "").trim();
        return `<blockquote>${content}</blockquote>`;
      });
      text = text.replace(/(?:^\s*\|.*\|\s*(?:\r?\n|$))+/gm, (match) => {
        const lines = match.trim().split(/\r?\n/).map((l) => l.trim()).filter(Boolean);
        if (lines.length < 2) return match;
        let html2 = '<div class="docmd-ai-table-wrap"><table>';
        let inBody = false;
        lines.forEach((line, idx) => {
          if (/^\|(?:\s*:?-+:?\s*\|)+$/.test(line)) return;
          const cells = line.split("|").slice(1, -1).map((c) => c.trim());
          if (cells.length === 0) return;
          if (idx === 0) {
            html2 += "<thead><tr>";
            cells.forEach((c) => {
              html2 += `<th>${c}</th>`;
            });
            html2 += "</tr></thead>";
          } else {
            if (!inBody) {
              html2 += "<tbody>";
              inBody = true;
            }
            html2 += "<tr>";
            cells.forEach((c) => {
              html2 += `<td>${c}</td>`;
            });
            html2 += "</tr>";
          }
        });
        if (inBody) html2 += "</tbody>";
        html2 += "</table></div>";
        return html2;
      });
      text = text.replace(/^\s*(?:-{3,}|\*{3,}|_{3,})\s*$/gm, "<hr />");
      text = text.replace(/\*\*([^*]+)\*\*/g, "<strong>$1</strong>");
      text = text.replace(/\*([^*]+)\*/g, "<em>$1</em>");
      text = text.replace(/`([^`]+)`/g, "<code>$1</code>");
      text = text.replace(/\[([^\]]+)\]\(([^)]+)\)/g, (_m, label, link) => {
        const finalUrl = resolveCanonicalUrl(link.trim());
        return `<a href="${finalUrl}" target="_blank" rel="noopener">${label}</a>`;
      });
      const blocks = text.split(/\n{2,}/);
      const html = blocks.map((block) => {
        const trimmed = block.trim();
        if (!trimmed) return "";
        if (/^<(?:h3|h4|h5|ul|ol|pre|blockquote|div|table|hr)|__CODE_BLOCK_/i.test(trimmed)) {
          return trimmed.replace(/\n/g, " ");
        }
        return `<p>${trimmed.replace(/\n/g, "<br/>")}</p>`;
      }).join("");
      let finalHtml = html;
      codeBlocks.forEach((cb, i) => {
        finalHtml = finalHtml.replace(`__CODE_BLOCK_${i}__`, cb);
      });
      const isLocal = typeof location !== "undefined" && (location.hostname === "localhost" || location.hostname === "127.0.0.1");
      const subpathBase = (cfg.siteBase || "").replace(/^\/|\/$/g, "");
      if (isLocal && subpathBase && !location.pathname.startsWith("/" + subpathBase)) {
        finalHtml = finalHtml.replace(new RegExp(`(https?://[^/\\s]+)?/${subpathBase}(/|\\b)`, "g"), (_m, origin) => {
          return (origin || "") + "/";
        });
      }
      return finalHtml;
    }
  };
  if (typeof document !== "undefined") {
    if (document.readyState === "loading") {
      document.addEventListener("DOMContentLoaded", () => new DocmdAIAssistantUI());
    } else {
      new DocmdAIAssistantUI();
    }
  }
  return __toCommonJS(index_exports);
})();