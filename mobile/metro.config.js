const { getDefaultConfig } = require('expo/metro-config');

const config = getDefaultConfig(__dirname);

// Resolve @/ → project root so Metro can find @/hooks/..., @/components/..., etc.
config.resolver.extraNodeModules = {
  ...(config.resolver.extraNodeModules || {}),
  '@': __dirname,
};

module.exports = config;
