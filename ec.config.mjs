// Expressive Code (Starlight's code block renderer) configuration.
import { pluginLineNumbers } from '@expressive-code/plugin-line-numbers';

/** @type {import('@astrojs/starlight/expressive-code').StarlightExpressiveCodeOptions} */
export default {
	plugins: [pluginLineNumbers()],
	defaultProps: { showLineNumbers: false },
	// Tabs render 4 columns wide (Visual Studio default; default here is 2), so imported sources that use
	// tabs line up with space-indented code.
	tabWidth: 4,
};
