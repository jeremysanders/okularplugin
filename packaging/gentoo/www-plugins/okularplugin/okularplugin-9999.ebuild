# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=4

inherit kde4-base nsplugins git-2

DESCRIPTION="Plugin for embedding okular into the browser."
HOMEPAGE="https://github.com/afrimberger/okularplugin"

EGIT_REPO_URI="git://github.com/afrimberger/okularplugin.git"
if [[ "${PV}" != "9999" ]]; then
	EGIT_TAG="v${PV}"
fi
SRC_URI=""

LICENSE="GPL"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND="kde-base/okular"
RDEPEND="${DEPEND}"

src_install() {
	kde4-base_src_install

	kde_libdir=`kde4-config --install lib`
	inst_plugin ${kde_libdir}/lib${PN}.so
}

pkg_postinst() {
	elog "okularplugin is now installed, but must be configured as PDF viewer"
	elog "in your browser(s)."
	elog "Chrome:"
	elog "   Open the address chrome://plugins, disable \"Chrome PDF viewer\""
	elog "   and check \"Always allowed\" for \"Okular plugin\"."
	elog "Chromium:"
	elog "   Open the address chrome://plugins and check \"Always allowed\""
	elog "   for \"Okular plugin\"."
	elog "Firefox:"
	elog "   Open the Preferences dialog, go in the Applications section and"
	elog "   select \"Use Okular plugin (in Firefox)\" beside"
	elog "   \"Portable Document Format (PDF)\"."
}
