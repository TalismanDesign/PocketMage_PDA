export default {
  title: "PocketMage",
  url: "https://talismandesign.github.io/PocketMage_PDA/docs",
  logo: { alt: "PocketMage", href: "./" },
  favicon: "/assets/favicon.png",
  theme: {
    name: "ruby",
    defaultMode: "system",
    enableModeToggle: true,
    positionMode: "top",
    codeHighlight: true,
    customCss: ["/assets/css/theme.css"],
    copyWidgets: {
      enabled: true,
      raw: true,
      context: true,
    },
  },
  layout: {
    footer: {
      style: "complete",
      description: "A clamshell PDA powered by an ESP32-S3 with E-Ink and OLED displays.",
      branding: true,
      columns: [
        {
          title: "Device",
          links: [
            { text: "Command Manual", url: "./command-manual/" },
            { text: "Features", url: "./features/" },
            { text: "FAQ", url: "./faq/" },
          ],
        },
        {
          title: "Build",
          links: [
            { text: "Architecture", url: "./development/architecture/" },
            { text: "Build Environments", url: "./development/build-environments/" },
            { text: "Making Apps", url: "./guides/making-apps/" },
            { text: "SDK docs", url: "https://talismandesign.github.io/PocketMage_SDK/docs" },
          ],
        },
        {
          title: "Community",
          links: [
            { text: "GitHub", url: "https://github.com/TalismanDesign/PocketMage_PDA" },
            { text: "Discord", url: "https://discord.gg/KSCapSf4XH" },
            { text: "Website", url: "https://pocketmage.org/" },
          ],
        },
      ],
    },
  },
  plugins: {
    search: {
      semantic: true,
      showConfidence: true,
    },
    seo: {
      defaultDescription:
        "PocketMage is an open-source clamshell PDA powered by an ESP32-S3 with E-Ink and OLED displays. Notes, calendar, tasks, and more.",
      openGraph: { defaultImage: "/assets/images/og-image.png" },
      twitter: { cardType: "summary_large_image" },
    },
    sitemap: {
      defaultChangefreq: "weekly",
      defaultPriority: 0.8,
    },
    mermaid: {},
    git: {},
    llms: {
      fullContext: true,
    },
  },
  search: true,
  minify: true,
  autoTitleFromH1: true,
  copyCode: true,
  pageNavigation: true,
  navigation: [
    { title: "Home", path: "/", icon: "home" },
    {
      title: "Get Started",
      icon: "rocket",
      collapsible: true,
      path: "/getting-started/index",
      children: [
        { title: "What is PocketMage", path: "/getting-started/what-is-pocketmage", icon: "info" },
        { title: "Features", path: "/features/index", icon: "star" },
        { title: "Format a MicroSD Card", path: "/tutorials/format-micro-sd", icon: "card-sd" },
      ],
    },
    {
      title: "Using the Device",
      icon: "keyboard",
      collapsible: true,
      path: "/command-manual/index",
      children: [
        { title: "Command Manual", path: "/command-manual/index", icon: "keyboard" },
        { title: "FAQ", path: "/faq/index", icon: "help-circle" },
      ],
    },
    {
      title: "Architecture",
      icon: "cpu",
      collapsible: true,
      path: "/development/index",
      children: [
        { title: "PocketMageOS overview", path: "/development/index", icon: "cpu" },
        { title: "System architecture", path: "/development/architecture", icon: "layers" },
        { title: "System State", path: "/reference/system-state", icon: "settings" },
      ],
    },
    {
      title: "Building & Flashing",
      icon: "settings",
      collapsible: true,
      path: "/development/build-environments",
      children: [
        { title: "Build Environments", path: "/development/build-environments", icon: "settings" },
      ],
    },
    {
      title: "Apps & Scripting",
      icon: "package",
      collapsible: true,
      path: "/guides/index",
      children: [
        { title: "Making Apps", path: "/guides/making-apps", icon: "code" },
        { title: "Native Apps", path: "/guides/native-apps", icon: "terminal" },
        { title: "ELF Apps", path: "/guides/elf-apps", icon: "package" },
        { title: "Scripting overview", path: "/scripting/index", icon: "terminal" },
        { title: "Basic Input/Output", path: "/scripting/example-c", icon: "terminal" },
        { title: "E-Ink Drawing", path: "/scripting/ink-c", icon: "image" },
        { title: "OLED Drawing", path: "/scripting/oled-c", icon: "monitor" },
        { title: "Full Command Reference", path: "/scripting/fullPotionCommandList", icon: "book" },
      ],
    },
    {
      title: "Reference",
      icon: "book",
      collapsible: true,
      path: "/reference/index",
      children: [
        { title: "App API", path: "/reference/app-api", icon: "code" },
        { title: "PocketMage Library", path: "/reference/pocketmage-library", icon: "box" },
        { title: "System State", path: "/reference/system-state", icon: "settings" },
      ],
    },
    {
      title: "SDK docs",
      path: "https://talismandesign.github.io/PocketMage_SDK/docs",
      icon: "boxes",
      external: true,
    },
    {
      title: "GitHub",
      path: "https://github.com/TalismanDesign/PocketMage_PDA",
      icon: "github",
      external: true,
    },
  ],
  footer: "Built with [docmd](https://docmd.io). [View on GitHub](https://github.com/TalismanDesign/PocketMage_PDA).",
  editLink: {
    enabled: true,
    baseUrl: "https://github.com/TalismanDesign/PocketMage_PDA/edit/main/Docs/docs",
    text: "Edit this page",
  },
};
